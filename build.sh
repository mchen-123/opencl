#!/bin/bash

rm -rf build
mkdir -p build && cd build && cmake .. && make

cp  tests/test_* ../

# 安装 runtime_mock 到 /opt/thrive/lib
# sudo mkdir -p /opt/thrive/lib
# sudo cp runtime_mock/libruntime_mock.so /opt/thrive/lib/
