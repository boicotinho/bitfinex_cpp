#!/bin/bash
CFG=${1:-Release}
rm -rf build_${CFG}
./vbuild.sh ${CFG}
cd build_${CFG}
bitfinex_test/bitfinex_test
