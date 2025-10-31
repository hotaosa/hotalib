# hotalib

Header-only C++23 algorithm library and notes for solving AtCoder-style problems.

## Overview

- The reusable library lives under `hotaosa/` and stays header-only for drop-in use during contests.
- Performance and APIs target AtCoder constraints; call out any exceptions directly in the code or docs.
- Repository discussions and coordination details are maintained in `AGENTS.md`.

## Quick Start

```bash
# Build the full library (useful to warm up build caches)
bazel build //hotaosa:all

# Run a GoogleTest target once added
bazel test //hotaosa:<target_name>
```

## Project Layout

- `hotaosa/` — Header-only library modules written in C++23.
- `notes/` — Contest knowledge, solution write-ups, and related research.
- `testdata/` — Reusable inputs mirrored after the module layout.
- `template/` — Contest-ready source templates.
- `tools/` — Utilities that help set up contest environments locally.
- `third_party/` — External dependencies vendored for builds.

## Development Guidelines

- Follow the Google C++ Style Guide with the stricter rules documented in `AGENTS.md` (header-only delivery, modern C++23 features).
- Keep `.clangd`, `.clang-tidy`, and formatting configs in sync with the style guide.
- Every change in `hotaosa/` requires accompanying GoogleTest-based tests and representative test data where practical.
- Document algorithmic complexity and any portability caveats in headers.

For deeper collaboration practices or open action items, see `AGENTS.md`.
