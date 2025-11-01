# AGENTS Collaboration Guide

## Project Snapshot

- **Domain focus**: Reusable algorithm library in `hotaosa/` for contest problem solving.
- **Primary goals**: Keep the `hotaosa/` library header-only and contest-ready.

## Workflow Overview

### Communication Protocol

- **Status updates**: Share concise progress notes in the chat whenever a planned step completes or a risk emerges; reference file paths or commands for traceability.
- **Blocking issues**: Raise blockers immediately via chat; request escalated commands with justification when sandbox limits interfere.
- **Tone & language**: Use Japanese for chat dialogue; keep repository artifacts (code, docs, comments) in English unless an exception is agreed.

## Coding Standards & Constraints

- **Languages & versions**: C++23 header-only templates. This is stricter than the Google C++ Style Guide baseline because we enforce header-only delivery (extra ODR/include hygiene) and lean on newer C++23 features; highlight any portability risks those introduce.
- **Build system**: Bazel (capture targets and helper scripts as they solidify).
- **Style guides**: Follow the Google C++ Style Guide; keep `.clangd` settings aligned with it and note any intentional deviations.
- **`hotaosa/` library**: Must remain header-only to fit contest workflows; implementations should stay readable without compromising contest-level performance and must ship with appropriate tests.
- **Performance expectations**: Guard asymptotic complexity and constant factors that satisfy AtCoder limits; note deviations explicitly.
- **External libraries**: Leverage libraries available on AtCoder judge servers when they raise quality (e.g., Abseil, Boost, Eigen); document usage and ensure build rules pull them consistently.
- **Comments & docs**: Keep explanatory comments concise, focused on contest-relevant tips; when documenting APIs, note the key computational complexity (e.g., `O(N)`).

## Testing Expectations

- **Minimum test coverage**: Every change in `hotaosa/` requires targeted tests (unit or scenario-based) to validate correctness.
- **Test data sources**: Store reusable inputs under a dedicated `testdata/` directory (mirror module layout, e.g., `testdata/hotaosa/<module>/`).
- **Test code location**: Co-locate test files alongside their headers in `hotaosa/` for quick traceability; update `BUILD.bazel` targets accordingly.
- **Test framework**: Standardize on GoogleTest via Bazel `cc_test`; share common fixtures/utilities to avoid duplication across modules.

## References

- **Style references**:
  - Google C++ Style Guide — <https://google.github.io/styleguide/cppguide.html>
  - Clang-Format Style Options — <https://clang.llvm.org/docs/ClangFormatStyleOptions.html> (align with `.clang-format`)
  - Bazel C++ Rules — <https://bazel.build/reference/be/c-cpp>
  - GoogleTest Primer — <https://google.github.io/googletest/primer.html>
- **AtCoder resources**:
  - Library bundle (ACL) — <https://atcoder.github.io/ac-library/>
