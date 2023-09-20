#!/bin/sh
cd ~/innoextract-wasm/tests/
exec python3 -m robot --outputdir output --logtitle "Task log" tests/smoke_tests.robot
