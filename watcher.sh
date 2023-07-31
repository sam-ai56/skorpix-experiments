#! /bin/bash

while true; do
    inotifywait -e modify -r . && ./build_and_run.sh "$@"
done