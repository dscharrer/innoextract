#!/bin/sh

#date and version number taken from .version file
date=$(cat "/home/runner/build/.version" | awk '{ print $1 }')
version=$(cat "/home/runner/build/.version" | awk '{ print $2 }')

echo "Date of the build: $date"
echo "Current build version: $version"

cp -r /home/runner/innoextract-wasm/tests/output /home/runner/build
cd /home/runner/build/output && ln -s log.html index.html
sed -i "s,\"title\":\"Task log,\"title\":\"Task log for build: "$version",g" /home/runner/build/output/log.html


mkdir -p "/home/runner/logs/$date-$version"
cp -rf /home/runner/build/output/* "/home/runner/logs/$date-$version"
echo "Done"
