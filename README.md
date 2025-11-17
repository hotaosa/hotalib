# hotalib

Header-only C++23 library for solving AtCoder problems.

## Overview

- The reusable library lives under `hotaosa/` and stays header-only for drop-in use during contests.
- Performance and APIs target AtCoder constraints; call out any exceptions directly in the code or docs.

## Quick Start

```bash
# Build the library (useful to warm up build caches)
bazel build //hotaosa:all

# Run all tests under hotaosa/
bazel test //hotaosa/...
```

## Project Layout

- `hotaosa/` — Header-only library modules written in C++23.
- `template/` — Contest-ready source templates.
- `tools/` — Utilities that help set up contest environments locally.
- `third_party/` — External dependencies vendored for builds (e.g. [AC Library](https://github.com/atcoder/ac-library)).
