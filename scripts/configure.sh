executable_dir=$(python -c 'import sys; print(sys.executable)')
kratos_install_dir=$(python -c 'import site; print(site.getsitepackages()[0])')
environment_path=$(dirname $executable_dir)
environment_path=$(dirname $environment_path)

cmake -B"../build"                              \
-DKRATOS_SOURCE_DIR=${KRATOS_PATH}              \
-DKRATOS_LIBRARY_DIR=${kratos_install_dir}/libs \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON              \
-DCMAKE_INSTALL_PREFIX=${environment_path}      \
..

cd ../build
make install
