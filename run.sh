#!/bin/bash

build_dir=build
executable=ipen

cmake -B $build_dir -DCMAKE_BUILD_TYPE=Debug &&
  cmake --build $build_dir &&
  $build_dir/$executable
