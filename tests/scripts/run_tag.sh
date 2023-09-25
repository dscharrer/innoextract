#!/bin/bash
echo "Chosen option: $1"
cd ~/innoextract-wasm/tests/
exec python3 -m robot --outputdir output --logtitle "Task log" -i $1 tests
