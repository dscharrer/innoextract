#!/usr/bin/env python3
# SPDX-License-Identifier: Zlib
"""Build synthetic format fixtures with pinned, caller-supplied Inno compilers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

SOURCE = Path(__file__).resolve().parent / 'fixtures'
SOURCE_FILES = ('cases.json', 'cases.txt', 'compilers.json', 'expected.json',
                'format.iss', 'payload/hello.txt', 'payload/unicode.txt')

def sha(path):
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()

def make_binary(size):
    return bytes(((i * 73) + (i // 251)) & 255 for i in range(size))

def load(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler-map', type=Path, required=True,
                        help='JSON mapping compiler keys to ISCC executable paths')
    parser.add_argument('--work-dir', type=Path, required=True,
                        help='Fresh build directory; use a generic path for public fixtures')
    parser.add_argument('--output-dir', type=Path, required=True,
                        help='Fresh compiled fixture directory, outside the source checkout')
    args = parser.parse_args()
    if os.name != 'nt':
        parser.error('Building the fixtures requires the Windows Inno compiler')
    if args.work_dir.exists() or args.output_dir.exists():
        parser.error('Choose fresh work and output directories')
    mapping = load(args.compiler_map)
    compilers = load(SOURCE / 'compilers.json')['compilers']
    cases = load(SOURCE / 'cases.json')['cases']
    if not cases:
        parser.error('No matching cases')
    args.work_dir.mkdir(parents=True)
    args.output_dir.mkdir(parents=True)
    expected = load(SOURCE / 'expected.json')['profiles']
    manifest = {'schemaVersion': 1, 'sources': {name: sha(SOURCE / name) for name in SOURCE_FILES},
                'cases': []}
    for case in cases:
        key = case['compiler']
        compiler = Path(mapping[key]).resolve()
        if sha(compiler) != compilers[key]['compilerSha256']:
            raise RuntimeError('Unexpected compiler SHA-256 for ' + key)
        work = args.work_dir / case['id']
        payload = work / 'payload'
        payload.mkdir(parents=True)
        for name in ('hello.txt', 'unicode.txt'):
            shutil.copyfile(SOURCE / 'payload' / name, payload / name)
        size = 9000000 if case['spanning'] else 8192
        (payload / 'data.bin').write_bytes(make_binary(size))
        shutil.copyfile(SOURCE / 'format.iss', work / 'format.iss')
        for path in work.rglob('*'):
            if path.is_file():
                os.utime(str(path), (1735689600, 1735689600))
        profile = 'spanning' if case['spanning'] else 'single'
        inputs = {'app/hello.txt': payload / 'hello.txt',
                  'app/nested/data.bin': payload / 'data.bin',
                  'app/unicodé/naïve.txt': payload / 'unicode.txt',
                  'app/architecture32.txt': payload / 'hello.txt',
                  'app/architecture64.txt': payload / 'hello.txt'}
        for wanted in expected[profile]:
            source = inputs[wanted['path']]
            if source.stat().st_size != wanted['bytes'] or sha(source) != wanted['sha256']:
                raise RuntimeError('Payload recipe differs from checked-in expectations')
        output = args.output_dir / case['id']
        output.mkdir()
        major, minor, patch = (int(x) for x in case['version'].split('.'))
        series = major * 10000 + minor * 100 + patch
        definitions = ('#define CaseId "' + case['id'] + '"\n'
                       '#define CompilerSeries ' + str(series) + '\n'
                       '#define Span ' + str(int(case['spanning'])) + '\n'
                       '#define SetupArch "' + case['architecture'] + '"\n')
        recipe = (SOURCE / 'format.iss').read_text(encoding='utf-8-sig')
        (work / 'format.iss').write_text(definitions + recipe, encoding='utf-8-sig')
        command = [str(compiler), '/Qp', '/O' + str(output.resolve()), str(work / 'format.iss')]
        result = subprocess.run(command, cwd=str(work), stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, timeout=120,
                                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        (work / 'compiler.stdout.txt').write_bytes(result.stdout)
        (work / 'compiler.stderr.txt').write_bytes(result.stderr)
        if result.returncode:
            print('\n'.join((result.stdout + result.stderr).decode('utf-8', errors='replace').splitlines()[-40:]))
            raise RuntimeError('Fixture compilation failed: ' + case['id'])
        setup = output / 'setup.exe'
        data = setup.read_bytes()
        pe = struct.unpack_from('<I', data, 0x3c)[0]
        machine = struct.unpack_from('<H', data, pe + 4)[0]
        expected_machine = 0x8664 if case['architecture'] == 'x64' else 0x14c
        if machine != expected_machine:
            raise RuntimeError('Unexpected setup architecture: ' + case['id'])
        if case['dataSignature'].encode('ascii') not in data:
            raise RuntimeError('Expected data format signature missing: ' + case['id'])
        files = [{'path': path.relative_to(args.output_dir).as_posix(),
                  'bytes': path.stat().st_size, 'sha256': sha(path)}
                 for path in sorted(output.iterdir()) if path.is_file()]
        if case['spanning'] and len([f for f in files if f['path'].endswith('.bin')]) < 2:
            raise RuntimeError('Disk-spanning fixture did not create multiple slices')
        licenses = args.output_dir / 'licenses'
        licenses.mkdir(exist_ok=True)
        license_source = compiler.parent / 'license.txt'
        shutil.copyfile(license_source, licenses / (key + '.txt'))
        manifest['cases'].append({'id': case['id'], 'version': case['version'],
                                  'compiler': key, 'architecture': case['architecture'],
                                  'dataSignature': case['dataSignature'], 'files': files})
        (args.output_dir / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
        print(case['id'] + ': built (' + str(len(files)) + ' package files)', flush=True)
    print('Built ' + str(len(cases)) + ' fixtures; installers were not executed.')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
