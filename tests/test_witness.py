import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT=Path(__file__).resolve().parents[1]/"scripts"/"verify_replay.py"

class ReceiptTests(unittest.TestCase):
    def test_failed_rerun_replaces_stale_pass(self):
        with tempfile.TemporaryDirectory() as tmp:
            dest=Path(tmp)/"witness.json"
            dest.write_text('{"status":"PASS"}')
            ret=subprocess.run([sys.executable,str(SCRIPT),
                                "--binary",str(Path(tmp)/"no-binary"),
                                "--out",str(dest)],capture_output=True,check=False)
            self.assertEqual(ret.returncode,1)
            result=json.loads(dest.read_text())
            self.assertEqual(result["status"],"FAIL")
            self.assertNotIn("PASS",dest.read_text())
            self.assertIn("error",result)
