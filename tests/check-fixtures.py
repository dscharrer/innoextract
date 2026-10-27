#!/usr/bin/env python3
# SPDX-License-Identifier: Zlib
"""Check synthetic Setup packages by invoking innoextract; never run Setup."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import zlib

SOURCE = Path(__file__).resolve().parent / 'fixtures'


def load(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def checked_path(root, name):
    path = (root / name).resolve()
    path.relative_to(root.resolve())
    return path


class Checks:
    def __init__(self, args):
        self.args = args
        args.output.mkdir(parents=True, exist_ok=True)
        self.output = Path(tempfile.mkdtemp(prefix='format-', dir=str(args.output)))
        self.rows = []

    def run(self, name, setup, options, expected_error=None):
        prefix = self.output / name
        command = self.args.wrapper + [str(self.args.extractor.resolve())] + options + [str(setup)]
        try:
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    timeout=30, creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
            Path(str(prefix) + '.stdout.txt').write_bytes(result.stdout)
            Path(str(prefix) + '.stderr.txt').write_bytes(result.stderr)
            output = (result.stdout + result.stderr).decode('utf-8', errors='replace')
            if expected_error is None:
                passed = result.returncode == 0 and 'warning' not in output.lower()
            else:
                passed = (0 < result.returncode < 128 and
                          (not expected_error or expected_error in output.lower()))
            row = {'case': name, 'passed': passed, 'exitCode': result.returncode}
            if expected_error is not None:
                row['expectedDiagnostic'] = expected_error
        except subprocess.TimeoutExpired as error:
            Path(str(prefix) + '.stdout.txt').write_bytes(error.stdout or b'')
            Path(str(prefix) + '.stderr.txt').write_bytes(error.stderr or b'')
            row = {'case': name, 'passed': False, 'error': 'timeout'}
        self.rows.append(row)
        return row

    def positive(self, case, expected):
        name = case['id']
        setup = self.args.fixtures / name / 'setup.exe'
        output = self.output / (name + '-files')
        row = self.run(name + '-extract', setup,
                       ['--no-extract-unknown', '-d', str(output)])
        actual = []
        if output.exists():
            actual = [{'path': p.relative_to(output).as_posix(), 'bytes': p.stat().st_size,
                       'sha256': sha(p)} for p in output.rglob('*') if p.is_file()]
        row['payloadsMatch'] = sorted(actual, key=lambda f: f['path']) == sorted(expected, key=lambda f: f['path'])
        row['passed'] = row['passed'] and row['payloadsMatch']
        self.run(name + '-checksums', setup, ['--test', '--no-extract-unknown'])

    def negative(self):
        for case_id in ('inno-6.5.0', 'inno-6.7.0', 'inno-7.1.0-x86'):
            original = (self.args.fixtures / case_id / 'setup.exe').read_bytes()
            pos = -1
            while True:
                pos = original.find(b'Inno Setup Setup Data (', pos + 1)
                if pos < 0:
                    raise RuntimeError('No CRC-verified encryption header: ' + case_id)
                header = pos + 64
                if header + 53 <= len(original) and zlib.crc32(original[header + 4:header + 53]) == struct.unpack_from('<I', original, header)[0]:
                    break
            bad_crc = bytearray(original)
            bad_crc[header] ^= 1
            bad_block = bytearray(original)
            bad_block[header + 53] ^= 1
            full = bytearray(original)
            full[header + 4] = 2
            struct.pack_into('<I', full, header, zlib.crc32(full[header + 4:header + 53]))
            mutations = [('encryption-crc', bad_crc, 'encryption header crc32 mismatch'),
                         ('truncated-header', original[:header + 52], ''),
                         ('block-crc', bad_block, 'block header crc32 mismatch'),
                         ('full-encryption', full, 'encrypted setup headers are not supported')]
            if case_id != 'inno-6.5.0':
                invalid_size = bytearray(original)
                block = header + 53
                struct.pack_into('<q', invalid_size, block + 4, -1)
                struct.pack_into('<I', invalid_size, block, zlib.crc32(invalid_size[block + 4:block + 13]))
                mutations.append(('negative-block-size', invalid_size, 'invalid stored block size'))
            for suffix, data, diagnostic in mutations:
                name = case_id + '-' + suffix
                target = self.output / (name + '.exe')
                target.write_bytes(data)
                self.run(name, target, ['--test', '--no-extract-unknown'], diagnostic)
        name = 'inno-6.5.2-bad-slice-magic'
        damaged = self.output / name
        shutil.copytree(self.args.fixtures / 'inno-6.5.2-spanning', damaged)
        first = damaged / 'setup-1.bin'
        data = bytearray(first.read_bytes())
        if data[:8] != b'idskb32\x1a':
            raise RuntimeError('Unexpected slice layout')
        data[:8] = b'INVALID!'
        first.write_bytes(data)
        self.run(name, damaged / 'setup.exe', ['--test', '--no-extract-unknown'], 'bad slice magic number')

    def finish(self):
        summary = {'checkCount': len(self.rows), 'passed': sum(row['passed'] for row in self.rows),
                   'failed': sum(not row['passed'] for row in self.rows), 'checks': self.rows}
        (self.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n', encoding='utf-8')
        for row in self.rows:
            print(row['case'] + ': ' + ('PASS' if row['passed'] else 'FAIL'))
        print('{passed}/{checkCount} checks passed'.format(**summary))
        return int(summary['failed'] != 0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--extractor', type=Path, required=True)
    parser.add_argument('--fixtures', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--case', dest='case_id')
    parser.add_argument('--negative', action='store_true', help='Run only malformed-input checks')
    parser.add_argument('--positive-only', action='store_true')
    parser.add_argument('--wrapper', action='append', default=[], help='Executable wrapper argument (repeat as needed)')
    args = parser.parse_args()
    if args.negative and (args.case_id or args.positive_only):
        parser.error('--negative cannot be combined with --case or --positive-only')
    args.fixtures = args.fixtures.resolve()
    cases = load(SOURCE / 'cases.json')['cases']
    ids = [case['id'] for case in cases]
    if ids != (SOURCE / 'cases.txt').read_text(encoding='utf-8').splitlines():
        raise RuntimeError('Case catalog and CMake case list differ')
    if args.case_id and args.case_id not in ids:
        parser.error('Unknown case')
    manifest = load(args.fixtures / 'manifest.json')
    required_sources = ('cases.json', 'cases.txt', 'compilers.json', 'expected.json',
                        'format.iss', 'payload/hello.txt', 'payload/unicode.txt')
    if set(manifest['sources']) != set(required_sources):
        raise RuntimeError('Fixture source inventory differs')
    for name, digest in manifest['sources'].items():
        if sha(checked_path(SOURCE, name)) != digest:
            raise RuntimeError('Fixture source hash differs: ' + name)
    if [case['id'] for case in manifest['cases']] != ids:
        raise RuntimeError('Compiled fixture inventory differs')
    selected = [case for case in cases if not args.case_id or case['id'] == args.case_id]
    needed = set(case['id'] for case in selected) if not args.negative else {
        'inno-6.5.0', 'inno-6.7.0', 'inno-7.1.0-x86', 'inno-6.5.2-spanning'}
    for compiled in manifest['cases']:
        if compiled['id'] in needed:
            for entry in compiled['files']:
                path = checked_path(args.fixtures, entry['path'])
                if path.stat().st_size != entry['bytes'] or sha(path) != entry['sha256']:
                    raise RuntimeError('Compiled fixture hash differs: ' + entry['path'])
    checks = Checks(args)
    if not args.negative:
        profiles = load(SOURCE / 'expected.json')['profiles']
        for case in selected:
            checks.positive(case, profiles['spanning' if case['spanning'] else 'single'])
    if args.negative or (not args.case_id and not args.positive_only):
        checks.negative()
    return checks.finish()


if __name__ == '__main__':
    raise SystemExit(main())
