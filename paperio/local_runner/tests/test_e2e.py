import pytest
import subprocess
import glob

@pytest.mark.parametrize("arg", glob.glob('tests/e2e/*.visio.gz'))
def test(arg):
    ret = subprocess.run(["python3", "localrunner.py", "--no-gui", "--replay", arg])
    assert 0==ret.returncode
