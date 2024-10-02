#!/usr/bin/env python3
import subprocess

from config import *

__all__ = ["run_sanity_tests"]

unit_test_cmds = [
    "{active_build_dir}/bin/expr.test --run-all",
]


def run_sanity_tests(args):
    debug_result = True
    release_result = True
    if args.debug:
        print("Running the sanity tests for the debug version")
        config["active_build_dir"] = config["build_debug_dir"]
        debug_result = sanity_tests()

    if args.release:
        print("Running the sanity tests for the release version")
        config["active_build_dir"] = config["build_release_dir"]
        release_result = sanity_tests()

    return debug_result and release_result


def sanity_tests():
    if config["dry_run"]:
        return True

    passed = True
    for cmd in unit_test_cmds:
        cmd = cmd.format(active_build_dir=config["active_build_dir"])
        output = subprocess.run(
            cmd,
            cwd=config["root_dir"],
            shell=True,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        ).stdout.decode("utf-8")
        print(output)
        if "Failed" in output and "Failed tests: 0" not in output:
            passed = False

    return passed
