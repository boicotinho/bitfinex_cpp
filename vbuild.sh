#!/bin/bash
CFG=${1:-Release}
echo "Build configuration: ${CFG}"

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

mkdir -p build_${CFG}
cd build_${CFG}
cmake -DCMAKE_BUILD_TYPE=${CFG} ../
make
