# Synthetic format regression fixtures

This directory contains original synthetic inputs for testing innoextract. The
fixture sources, payload text, generator and checker use the repository's zlib
license. Generated installers contain Inno Setup runtime components; their
original license notices accompany the binary attachment under `licenses/`.

No application installation is required. The checker invokes **innoextract only**;
it never executes a fixture's `setup.exe` or embedded installation actions.

## Cases and expected results

`cases.json` describes twelve cases, also listed in `cases.txt` for CMake:

- A 6.2.2 compatibility control.
- Each changed layout: 6.4.2, 6.4.3, 6.5.0, 6.5.2, 6.6.0, 6.6.1 and 6.7.0.
- 7.1.0 x86 and x64 Setup executables (both use data signature 7.0.0.3).
- Uncompressed disk-spanning packages from 6.5.2 and 7.1.0 x64, with three slices each.

Every case extracts five files: text, deterministic binary data, a Unicode path,
and two explicitly selected file architectures. The common script also contains
nested components/tasks, directories, an icon, INI and registry entries, explicit
registry/run architectures, deletion entries and compiled Pascal code. Newer
compiler directives exercise the changed header fields; Inno 7 includes OnLog.
These entries exercise serialization and stream alignment. This is not a test of
installation behavior or recovery of every metadata field through the CLI.

`expected.json` contains lengths and SHA-256 hashes derived from the **input
payloads**, not an extractor's output. `hello.txt` and `unicode.txt` are UTF-8 with
LF line endings. The binary byte at index `i` is
`((i * 73) + (i // 251)) & 255`, for 8,192 bytes in ordinary cases and 9,000,000
bytes in spanning cases. Extraction requires the exact file inventory and bytes.
A separate `--test --no-extract-unknown` pass checks stored payload checksums.
Warnings fail positive checks.

Fifteen negative checks mutate copies of these packages: encryption-header CRC,
truncation, block-header CRC, unsupported full-header encryption, invalid signed
64-bit block size, and an invalid external slice identifier. Nonzero error exits
and applicable diagnostics are required. Timeouts and process crashes fail.

## Use the compiled fixture attachment

Download the [compiled fixture attachment](https://github.com/mplabsorg/innoextract/releases/download/format-fixtures-v1/innoextract-format-fixtures.zip)
and extract `innoextract-format-fixtures.zip` outside the source checkout. Point the
checker at the resulting `format-fixtures` directory containing `manifest.json`.
The archive SHA-256 is recorded in `prebuilt.json`. Its manifest records the exact
package files and hashes, plus fingerprints of the fixture recipe and inputs.
The checker verifies those fingerprints before testing. Keep Setup and its slice
files together. No compiler or installed application is needed to run the checks.

With Python 3.8 or newer and a built innoextract:

```sh
python tests/check-fixtures.py --extractor build/innoextract \
  --fixtures ../format-fixtures --output build/format-results
```

Use `build/innoextract.exe` on Windows, or a configuration directory when using a
multi-configuration generator. Use `--case inno-7.1.0-x64` for one positive case,
`--positive-only` for a baseline comparison, or `--negative` for malformed inputs.
An executable wrapper can be supplied as repeated `--wrapper=argument` options.
The checker creates a fresh results directory on every run, retaining stdout,
stderr and a machine-readable summary. It does not overwrite fixture packages.

To register twelve positive cases and one negative group with CTest:

```sh
cmake -S . -B build -DBUILD_TESTS=ON \
  -DINNOEXTRACT_TEST_FIXTURES_DIR=/absolute/path/to/format-fixtures
cmake --build build
ctest --test-dir build --output-on-failure
```

The existing unit test remains enabled. Python and the binary attachment are
optional dependencies: leaving the fixture directory unset retains unit tests
without format integration tests. `RUN_TARGET` is forwarded to the checker when
cross-compiling. The portable checker uses only Python's standard library; fixture
compilation requires Windows.

## Rebuild the packages

`compilers.json` records official package URLs, package SHA-256 values and ISCC
executable SHA-256 values. Obtain the listed compiler distributions and retain each
complete compiler directory, including `license.txt`. Copy
`compilers.example.json` to an external compiler-map file, then replace each value
with the corresponding ISCC path. Relative paths resolve from the invocation's
working directory. The builder verifies the ISCC executable before each compile.
Do not commit the compiler map, downloaded tool distributions or generated Setup
packages.

Use fresh, generic directories so machine-specific source paths do not enter the
compiled metadata. For example, from the source checkout on Windows:

```powershell
python tests/build-fixtures.py --compiler-map C:/fixture-tools/compilers.json `
  --work-dir C:/format-fixture-build --output-dir C:/format-fixtures
```

The builder generates payloads, checks them against `expected.json`, compiles all
cases and verifies their PE architecture and setup-data signature. It copies the
runtime license notices and writes `manifest.json`. Compiler diagnostics remain
in the work directory outside the binary attachment. Package hashes can differ
between rebuilds because compiler/runtime timestamps vary; payload bytes and
expected extraction results are reproducible. Keep fixture text files with LF
line endings as specified by `.gitattributes`.

To produce an attachment, archive the output directory as `format-fixtures/`
without work directories, toolchains or compiler logs. Recalculate `prebuilt.json`
when replacing the distributed archive. Independently rebuilt packages can be
used directly with the checker; their manifest supplies their own package hashes.

## Observed before/after results

Untouched upstream commit `6e9e34ed0876014fdb46e684103ef8c3605e382e` passes the
6.2.2 control and fails the other eleven positive cases. The updated extractor
passes all twelve: 60 files match the expected bytes and all twelve checksum
checks pass. All fifteen malformed-input checks also pass. With the existing
unit test, the configured CTest run passes 14/14 tests.

This compact set demonstrates the reported format changes. It does not establish
exhaustive compatibility with every Inno option or third-party variant. Fully
encrypted headers remain explicitly unsupported; file-only encryption, external
downloads, archive expansion and files larger than 4 GiB are outside this set.
