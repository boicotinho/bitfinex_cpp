#!/bin/bash
CFG=${1:-Release}
rm -rf build_${CFG}
./vbuild.sh ${CFG}
cd build_${CFG}
marketlinks/bitfinex/utest/bitfinex_utest
