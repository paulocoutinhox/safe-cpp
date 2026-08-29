#!/usr/bin/env python3

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BUILD = ROOT / "build"


def run(*command: str, env: dict[str, str] | None = None) -> None:
    subprocess.run(command, cwd=ROOT, check=True, env=env)


def source_files() -> list[str]:
    paths = [ROOT / "include", ROOT / "src", ROOT / "tests", ROOT / "examples"]
    return sorted(str(file) for path in paths for file in path.rglob("*") if file.suffix in {".hpp", ".cpp"})


def help_task() -> None:
    print("safe-cpp tasks")
    print("  help          Show this help")
    print("  configure     Configure a debug build with tests, examples and sanitizers")
    print("  build         Build the configured project")
    print("  test          Build and run the test suite")
    print("  format        Format C++ sources with clang-format")
    print("  format-check  Verify C++ formatting")
    print("  clean         Remove build output")
    print("  all           Configure, build and test")


def sanitizers_enabled() -> bool:
    cache = BUILD / "CMakeCache.txt"
    if not cache.exists():
        return False

    return "SAFE_CPP_ENABLE_SANITIZERS:BOOL=ON" in cache.read_text()


def sanitizer_environment() -> dict[str, str]:
    environment = os.environ.copy()
    leak_detection = "1" if sys.platform.startswith("linux") else "0"
    environment["ASAN_OPTIONS"] = f"halt_on_error=1:abort_on_error=1:detect_leaks={leak_detection}"
    environment["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    return environment


def configure() -> None:
    run(
        "cmake",
        "-S",
        ".",
        "-B",
        str(BUILD),
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DSAFE_CPP_BUILD_TESTS=ON",
        "-DSAFE_CPP_BUILD_EXAMPLES=ON",
        "-DSAFE_CPP_ENABLE_SANITIZERS=ON",
    )


def build() -> None:
    if not sanitizers_enabled():
        configure()

    run("cmake", "--build", str(BUILD), "--parallel")


def test() -> None:
    build()
    run("ctest", "--test-dir", str(BUILD), "--output-on-failure", env=sanitizer_environment())


def format_task(check: bool) -> None:
    executable = shutil.which("clang-format")
    if executable is None:
        raise SystemExit("clang-format is required")

    files = source_files()
    if check:
        run(executable, "--dry-run", "--Werror", *files)
        return

    run(executable, "-i", *files)


def clean() -> None:
    shutil.rmtree(BUILD, ignore_errors=True)


def all_task() -> None:
    configure()
    build()
    test()


TASKS = {
    "help": help_task,
    "configure": configure,
    "build": build,
    "test": test,
    "format": lambda: format_task(False),
    "format-check": lambda: format_task(True),
    "clean": clean,
    "all": all_task,
}


def main() -> int:
    task = sys.argv[1] if len(sys.argv) > 1 else "help"
    action = TASKS.get(task)
    if action is None:
        print(f"Unknown task: {task}", file=sys.stderr)
        help_task()
        return 2

    action()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
