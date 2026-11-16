# VACD 5.0 demonstration — arithmetic correctness patch

This change preserves the original author's collision-demo purpose and attribution while replacing unchecked fixed-point macros with small, inspectable functions. **Q16.16 is established prior art.**

## Reproduced concerns in source

- Original `INT_TO_FIXED(x)` multiplied signed integer operands without promoting first. Large values can overflow **before** the cast to `int32_t`.
- Original `FIXED_ADD(a,b)` used signed 32-bit addition without checking. Overflow is undefined behaviour in C++.
- Original `FIXED_MUL(a,b)` had a 64-bit intermediate, but silently truncated the result to signed 32 bits.
- Movement could partially update an entity if only a later arithmetic operation failed.
- No executable regression tests were included in the root demonstration.

The replacement checks ranges before narrowing, uses a 64-bit intermediate, enforces non-negative AABB extents, and makes each entity movement update atomic. It retains the original strict-overlap convention: touching edges do not collide.

## Verify with GCC or Clang

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all tests/test_vacd.cpp -o /tmp/vacd-test
/tmp/vacd-test
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all 'VACD 5 Demo.cpp' -o /tmp/vacd-demo
/tmp/vacd-demo
```

These tests check boundary overflow, signed multiplication, atomic movement, collision versus touching, and normal execution. They do **not** prove determinism on all targets, autonomy, sensor safety, or perpetual precision.

**Scope:** This PR does not modify or redistribute the third-party ZIP/tar archives in `Truth`, and does not claim the separate Vulkan sample is complete.

## QSOL-IMC VANILLAJS/OPT-inspired integration

This patch adapts **engineering contracts**, not a theorem or a runtime dependency.

- **VANILLAJS:** `QSOLKCB/VANILLAJS/modules/integer-math/index.js` (Git blob `d457b0154d9dfc8b50fd593b5e9dff75e1ea66b9`) demonstrates signed-int32 input checking, bounded operations, and rejection instead of wraparound. This project independently expresses the relevant checked-arithmetic contract in C++17. It does **not** claim to incorporate VANILLAJS's Lean proof or establish Lean/C++ refinement.
- **OPT:** `QSOLKCB/OPT/AGENTS.md` calls for an explicitly named reuse invariant and reference-equivalence tests. Here, **obstacle AABB is immutable during the simulation**, so the demo computes its bounds once and reuses them; `tests/test_vacd_replay.cpp` compares cached and uncached paths for 128 deterministic starting positions over 60 steps.
- **Replay witness:** A stdlib-only Python runner executes the demo twice, checks identical output and successful exit codes, and writes an atomic JSON receipt with SHA-256 of the trace. A failed rerun overwrites any previous PASS receipt. This is a *finite reproducibility witness*, not proof of all-target determinism or collision-model correctness.

No new dependencies or runtime archives are introduced. The only demonstrated optimization is avoiding repeated calls to `obstacle.bounds()` under the stated stationary-obstacle invariant; **no speed-up ratio is claimed without benchmark evidence**.

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -fsanitize=undefined -fno-sanitize-recover=all tests/test_vacd_replay.cpp -o /tmp/vacd-replay-test
/tmp/vacd-replay-test
python3 scripts/verify_replay.py --binary /tmp/vacd-demo --out build/vacd-replay-receipt.json
python3 -m unittest discover -s tests -p test_witness.py
```

## Review hardening — exact representation and fail-closed geometry

The public C++ API now wraps Q16.16 raw values in a **strong `fixed` type**. Whole integer values must use `from_integer(int64_t)`; raw scaled values must use `fixed::from_raw(int32_t)`. Implicit conversion from integer or floating-point primitives is prohibited by compile-time assertions. Inputs are range-checked before narrowing.

Geometry explicitly rejects inverted AABBs, excludes zero-area boxes, and preserves the existing non-overlap rule for merely touching edges. Both X and Y overflow tests ensure that an entity's two-position update is atomic.

C++ tests deliberately compile with `-DNDEBUG` and use always-active checks; `assert` alone would silently vanish in release builds. The reference-vs-cached replay suite similarly performs active comparisons under `NDEBUG`. The bound and limited test matrix remain unchanged.
