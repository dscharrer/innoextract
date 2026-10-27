# Inno Setup format updates through 7.1.0

These changes extend the setup-data readers beyond the 6.4.0 layout. The original license and attribution are retained, and the additions use the same zlib license.

## Format changes

The readers follow the official Inno Setup definitions in [Shared.Struct.pas](https://github.com/jrsoftware/issrc/blob/is-7_1_0/Projects/Src/Shared.Struct.pas) and the compiler serialization order. Setup-data version signatures can differ from release version numbers: Inno Setup 7.0.2 and 7.1.0 use the `7.0.0.3` data signature.

| Data version | Changes handled |
|---|---|
| 6.4.2 | Additional close-applications filter string |
| 6.4.3 | Compact file-location flags and removal of the Sign byte |
| 6.5.0 | CRC-protected encryption header, signature-key records, file verification/download/archive fields, loader table revision 2 and 64-bit loader offsets |
| 6.5.2 | 64-bit file-location offsets and disk-slice sizes, image background colors |
| 6.6.0 | Revised language records, wizard appearance settings and dark image sets |
| 6.6.1 | Wizard image opacity |
| 6.7.0 | 64-bit compressed-header sizes, additional strings and image sets, byte-sized component/task levels, shared x86/x64 flag padding |
| 7.0.0.3 | Compiled-code version, explicit file/registry/run bitness, run OnLog string |

The slice reader preserves 64-bit positions and bounds its magic-ID search. Compressed-block sizes are checked against the stream offset range. Header CRC checks remain active. The existing operating-system API baseline is unchanged.

## Validation

An external generated corpus contained 154 setup executables: 97 version smoke cases from Inno Setup 5.0.6 through 7.1.0, plus 19 feature cases each compiled with 6.2.2, 7.1.0 x86 and 7.1.0 x64. This corpus and its local manifests are not included in the source change.

The untouched upstream baseline at commit `6e9e34ed0876014fdb46e684103ef8c3605e382e` passed 99 cases. The updated readers passed all 154 cases with no extraction warnings. All 328 extracted payload files matched reference lengths and SHA-256 hashes, and all checksum tests passed with unknown formats disabled. Output path comparisons normalize the spelling of symbolic Inno constants; file contents are compared without normalization.

The stored-flag regression test covers the modern eight-byte boundary, legacy three-byte padding, 16-bit behavior and truncated padding. External negative checks cover bad encryption-header CRCs, truncated headers, bad block-header CRCs, unsupported full-header encryption and invalid signed 64-bit block sizes.

The public [synthetic fixture suite](../tests/fixtures/README.md) includes reproducible sources, pinned compiler provenance, independently derived payload hashes and a portable checker. Its twelve positive cases cover the changed layouts, Inno 7 x86/x64 and external disk slices. Untouched upstream passes the 6.2.2 control only; the updated readers pass all twelve cases (60 extracted files) and fifteen malformed-input checks. The optional CTest integration passes 14/14 tests including the existing unit test. Compiled fixtures are supplied as a separate attachment, outside Git.

Build and run the included unit tests using the existing CMake workflow:

```sh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Limits

Fully encrypted setup headers introduced in Inno Setup 6.5 are explicitly rejected. File-only encryption retains the existing XChaCha20 path but was not exercised by the generated corpus. Additional wizard image variants and embedded archive helpers are read or skipped for stream alignment rather than exposed as new CLI output.

This validation does not establish exhaustive support for third-party variants, external downloads, archive expansion or files larger than 4 GiB. Existing legacy format behavior is retained.
