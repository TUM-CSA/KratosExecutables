#!/usr/bin/env bash

if [[ -z "$KRATOS_PATH" ]]; then
    >&2 echo "Error: KRATOS_PATH is unset. Point it to the Kratos source directory."
    exit 1
fi

executable_dir=$(python -c 'import sys; print(sys.executable)')
kratos_install_dir=$(python -c 'import site; print(site.getsitepackages()[0])')
environment_path=$(dirname $executable_dir)
environment_path=$(dirname $environment_path)

script_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
build_dir="$script_dir/../build"

if ! mkdir -p "$build_dir"; then
    exit 1
fi

if ! cd "$script_dir/.."; then
    exit 1
fi

if ! cmake "-B$build_dir"                                   \
           -DKRATOS_SOURCE_DIR=${KRATOS_PATH}               \
           -DKRATOS_LIBRARY_DIR=${kratos_install_dir}/libs  \
           -DCMAKE_INSTALL_PREFIX=${environment_path}       \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON               \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo                \
           -DCMAKE_CXX_FLAGS=-fno-omit-frame-pointer; then
    exit 1
fi

if ! cmake --build "$build_dir" --target install -j4; then
    exit 1
fi

exit 0
