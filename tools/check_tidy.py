#!/usr/bin/env python3
"""clang-tidy execution lane.

Runs this repository's own .clang-tidy profile over its translation units and
gates on the correctness checks (the set listed in WarningsAsErrors in
.clang-tidy). Strictly read-only: it never applies fixes and never modifies
source files - formatting and style are authored, not derived.

Usage:
    python tools/check_tidy.py [options] [paths...]

Options:
  --build-dir DIR    Build tree holding compile_commands.json. When omitted it
                     is searched for in build/ninja, build, then any build/*,
                     then the repository root.
  --clang-tidy EXE   clang-tidy binary to use (default: clang-tidy on PATH).
  --jobs N           Translation units per clang-tidy invocation (default 8).
  --list             Print the translation units that would be analysed, exit.
  --verbose          Echo the clang-tidy command lines.

Positional paths restrict the run to matching files or directories.

Exit codes:
    0  clean
    1  findings - the correctness gate failed
    2  setup problem (no compilation database, or no clang-tidy)
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

# Directory segments that are never ours to analyse: build trees (which is
# where the fetched dependencies land) plus vendored and generated sources.
EXCLUDED_SEGMENTS = {
    ".git", "_deps", "build", "third_party", "thirdparty", "external",
    "vcpkg", "node_modules",
}

# clang-tidy emits "path:line:col: severity: message". The file field is
# non-greedy so a Windows drive letter ("C:\\...") is consumed correctly.
DIAGNOSTIC_RE = re.compile(
    r"^(?P<file>\S.*?):(?P<line>\d+):(?P<col>\d+): "
    r"(?P<severity>error|warning|note): (?P<msg>.*)$"
)


def parse_args(argv):
    p = argparse.ArgumentParser(
        prog="check_tidy.py",
        description="Run clang-tidy over this repository and gate on correctness findings.")
    p.add_argument("--build-dir", type=Path, default=None,
                   help="build tree containing compile_commands.json")
    p.add_argument("--clang-tidy", default="clang-tidy",
                   help="clang-tidy binary to use (default: clang-tidy)")
    p.add_argument("--jobs", type=int, default=8,
                   help="translation units per clang-tidy invocation (default 8)")
    p.add_argument("--list", action="store_true",
                   help="list the translation units that would be analysed, then exit")
    p.add_argument("--verbose", action="store_true",
                   help="echo the clang-tidy command lines")
    p.add_argument("paths", nargs="*",
                   help="restrict the run to these files or directories")
    return p.parse_args(argv)


def find_compile_commands(build_dir):
    """Return the directory holding compile_commands.json, or None."""
    candidates = []
    if build_dir is not None:
        candidates.append(Path(build_dir))
    else:
        candidates.append(REPO_ROOT / "build" / "ninja")
        candidates.append(REPO_ROOT / "build")
        candidates.extend(sorted(REPO_ROOT.glob("build/*")))
        candidates.append(REPO_ROOT)

    for cand in candidates:
        if (cand / "compile_commands.json").is_file():
            return cand
    return None


def is_ours(path):
    """True when a translation unit belongs to this repository's own code."""
    try:
        rel = path.resolve().relative_to(REPO_ROOT)
    except ValueError:
        return False
    return not any(seg in EXCLUDED_SEGMENTS for seg in rel.parts)


def matches_filters(path, filters):
    if not filters:
        return True
    text = str(path)
    return any(text.startswith(str(f)) or str(f) in text for f in filters)


def collect_units(compile_dir, filters):
    with open(compile_dir / "compile_commands.json", "r", encoding="utf-8") as fh:
        entries = json.load(fh)

    units = {}
    for entry in entries:
        raw = entry.get("file")
        if not raw:
            continue
        path = Path(raw)
        if not path.is_absolute():
            path = compile_dir / path
        if not is_ours(path):
            continue
        if not matches_filters(path, filters):
            continue
        units[str(path.resolve())] = path
    return [units[k] for k in sorted(units)]


def run_clang_tidy(exe, compile_dir, units, jobs, verbose):
    """Run clang-tidy in batches. Returns (errors, warnings, tool_failed)."""
    errors, warnings = [], []
    tool_failed = False
    batch_size = max(1, jobs)

    for start in range(0, len(units), batch_size):
        batch = units[start:start + batch_size]
        cmd = [exe, "-p", str(compile_dir)] + [str(u) for u in batch]
        if verbose:
            print("  $ " + " ".join(cmd))
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True,
                                  cwd=str(REPO_ROOT))
        except OSError as exc:
            print("error: could not run {}: {}".format(exe, exc), file=sys.stderr)
            return errors, warnings, True

        output = (proc.stdout or "") + (proc.stderr or "")
        for line in output.splitlines():
            m = DIAGNOSTIC_RE.match(line.strip())
            if not m:
                # "N warnings generated", "Suppressed N warnings", tool noise.
                continue
            record = "{}:{}: {}: {}".format(
                m.group("file"), m.group("line"), m.group("severity"), m.group("msg"))
            if m.group("severity") == "error":
                errors.append(record)
            elif m.group("severity") == "warning":
                warnings.append(record)

        # A non-zero status with no parsed error: usually the tool itself
        # failing (bad flags, missing header). Surface it rather than
        # reporting a false clean run.
        if proc.returncode != 0 and not errors:
            tool_failed = True
            tail = "\n".join(output.splitlines()[-12:])
            print("error: clang-tidy exited {} without diagnostics:".format(proc.returncode),
                  file=sys.stderr)
            print(tail, file=sys.stderr)

    return errors, warnings, tool_failed


def main(argv):
    args = parse_args(argv)

    if shutil.which(args.clang_tidy) is None:
        print("error: clang-tidy not found ({}); install LLVM/clang or pass --clang-tidy"
              .format(args.clang_tidy), file=sys.stderr)
        return 2

    compile_dir = find_compile_commands(args.build_dir)
    if compile_dir is None:
        print("error: compile_commands.json not found.", file=sys.stderr)
        print("       Configure with a Ninja or Makefile generator so the compilation "
              "database is exported, e.g.", file=sys.stderr)
        print("         cmake -S . -B build/ninja -G Ninja", file=sys.stderr)
        return 2

    filters = [Path(f) for f in args.paths]
    units = collect_units(compile_dir, filters)

    if not units:
        print("error: no translation units matched (from {})", file=sys.stderr)
        return 2

    print("clang-tidy: {} translation units from {}".format(len(units), compile_dir))

    if args.list:
        for u in units:
            print("  {}".format(u.relative_to(REPO_ROOT)))
        return 0

    errors, warnings, tool_failed = run_clang_tidy(
        args.clang_tidy, compile_dir, units, args.jobs, args.verbose)

    for w in warnings:
        print(w)
    for e in errors:
        print(e)

    print("")
    print("clang-tidy: {} error(s), {} warning(s) across {} translation units".format(
        len(errors), len(warnings), len(units)))
    if warnings:
        print("            (warnings report only; the gate is the WarningsAsErrors "
              "set in .clang-tidy)")

    if tool_failed:
        return 2
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
