#!/bin/sh

workflow=$1
run_no=$2
current_date=$(date +%s)
log_path="/home/runner/logs/$current_date--$workflow-#$run_no"

#date and version number taken from .version file
version_file="/home/runner/build/.version"
date=$(cat $version_file | awk '{ print $1 }')
version=$(cat $version_file | awk '{ print $2 }')

echo "Date of the build: $date"
echo "Current build version: $version"
echo "Log path: $log_path"


cp -r /home/runner/innoextract-wasm/tests/output /home/runner/build
cd /home/runner/build/output && ln -s log.html index.html
sed -i "s,\"title\":\"Task log,\"title\":\"Task log for build: "$version",g" /home/runner/build/output/log.html

mkdir -p $log_path
cp -rf /home/runner/build/output/* $log_path
cp $version_file $log_path
echo "Done."

