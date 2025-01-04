#!/bin/bash

executable=ipen

build_type=Debug
build_dir=build

release_mode=false

while getopts "r" flag; do
  case "${flag}" in
    r)
      release_mode=true
      build_dir=release
      build_type=Release
      ;;
  esac
done

cmake -B $build_dir -DCMAKE_BUILD_TYPE=$build_type

if $release_mode; then
  cmake --build $build_dir --config $build_type
  sudo cmake --install $build_dir
else
  echo "Building and running in debug mode"
    cmake --build $build_dir &&
    $build_dir/$executable
fi
