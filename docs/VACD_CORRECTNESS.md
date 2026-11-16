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
