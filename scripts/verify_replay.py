#!/usr/bin/env python3
"""OPT-style source-bound replay receipt for the standalone VACD demo.

Executed vs inferred are separate: a matching checksum demonstrates only
bit-identical stdout for two executions on this machine, with these inputs.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

def write_atomic(path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp=path.with_name(path.name+".tmp")
    tmp.write_text(json.dumps(payload, sort_keys=True, indent=2)+"\n", encoding="utf-8")
    os.replace(tmp, path)

def verify(binary, out):
    record={"kind":"qsol.vacd.replay.v1", "status":"NOT_RUN",
            "binary":str(binary), "checks":{}, "sha256":None}
    write_atomic(out,record)  # Stale PASS can never survive a rerun.
    try:
        # The executable bytes are an input to this finite witness.
        record["binary_sha256"]=hashlib.sha256(binary.read_bytes()).hexdigest()
        results=[]
        for attempt in range(2):
            proc=subprocess.run([str(binary)], capture_output=True,timeout=12,
                                check=False)
            record["checks"][f"execution_{attempt+1}"]={"exit_code":proc.returncode}
            if proc.returncode!=0:
                raise RuntimeError("demo exited nonzero: "+proc.stderr.decode("utf-8","replace")[:300])
            results.append(proc.stdout)
        # A changed executable invalidates the result even with matching stdout.
        if hashlib.sha256(binary.read_bytes()).hexdigest()!=record["binary_sha256"]:
            raise RuntimeError("binary changed during verification")
        if results[0]!=results[1]:
            raise RuntimeError("nonidentical stdout across deterministic replays")
        if b"COLLISION DETECTED!" not in results[0]:
            raise RuntimeError("expected collision witness absent")
        record["sha256"]=hashlib.sha256(results[0]).hexdigest()
        record["checks"]["identical_stdout"]=True
        record["checks"]["collision_seen"]=True
        record["status"]="PASS"
        return 0
    except (OSError, subprocess.TimeoutExpired, RuntimeError) as exc:
        record["status"]="FAIL"
        record["error"]=str(exc)[:500]
        return 1
    finally:
        write_atomic(out,record)

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--binary",type=Path,required=True)
    p.add_argument("--out",type=Path,required=True)
    args=p.parse_args()
    return verify(args.binary,args.out)

if __name__=="__main__":
    sys.exit(main())
