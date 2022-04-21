#!/bin/bash
CFG=${1:-Release}
echo "Build configuration: ${CFG}"

EXTRA_CMAKE_FLAGS=""

if [[ ! -z "${USE_ICC}" ]]; then
    ICC_ROOT="/opt/intel/compilers_and_libraries_2019.1.144/linux/bin/intel64"
    EXTRA_CMAKE_FLAGS+=" -DCMAKE_C_COMPILER=${ICC_ROOT}/icc"
    EXTRA_CMAKE_FLAGS+=" -DCMAKE_CXX_COMPILER=${ICC_ROOT}/icpc"
    #SET(CMAKE_C_COMPILER   ${ICC_ROOT}/icc)
    #SET(CMAKE_CXX_COMPILER ${ICC_ROOT}/icpc)
fi

PYTHON_TOOLSET=/opt/rh/rh-python36/enable
GCC_TOOLSET=/opt/rh/devtoolset-9/enable

if [[ -f "${PYTHON_TOOLSET}" ]]; then
    echo "Sourcing toolset ${PYTHON_TOOLSET}"
    source ${PYTHON_TOOLSET}
fi

if [[ -f "${GCC_TOOLSET}" ]]; then
    echo "Sourcing toolset ${GCC_TOOLSET}"
    source ${GCC_TOOLSET}
fi

echo "USING: $(python --version)"
echo "USING: $(gcc --version)"
echo "EXTRA_CMAKE_FLAGS: ${EXTRA_CMAKE_FLAGS}"

mkdir -p build_${CFG}
cd build_${CFG}
clear && nice cmake -DCMAKE_BUILD_TYPE=${CFG} ${EXTRA_CMAKE_FLAGS} ../ && clear && nice make -j
