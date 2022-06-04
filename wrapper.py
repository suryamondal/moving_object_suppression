#!/usr/bin/env python3
"""Convenience wrapper for SuppressObjects.C"""
import os
import sys
import subprocess

SCRIPT_PATH = os.path.abspath(__file__)
SCRIPT_DIR = os.path.dirname(SCRIPT_PATH)
SCRIPT_NAME = os.path.basename(SCRIPT_PATH)

if __name__ == "__main__":
    args = sys.argv[1:] if len(sys.argv) > 1 else []
    if len(args) != 4:
        print(__doc__)
        print(f"Usage: ./{SCRIPT_NAME} FrameDir LogFile")
    else:
        os.chdir(SCRIPT_DIR)
        macro = f'{args[0]}.C("{args[1]}","{args[2]}","{args[3]}")'
        cl = ['root', '-l', '-b', '-q', macro]
        print(cl)
        sys.exit(subprocess.run(cl).returncode)
