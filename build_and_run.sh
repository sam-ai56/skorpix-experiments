#! /bin/bash


mkdir build

cd build
cmake ..
make -j11
cd ..

killall $1

./build/$1 &