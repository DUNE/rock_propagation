#!/bin/bash
export ROCKPROP_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export ROCKPROP_BUILD_DIR=${ROCKPROP_DIR}/build
mkdir -p $ROCKPROP_BUILD_DIR
export PATH=$ROCKPROP_BUILD_DIR/bin:$PATH
echo "Top RockProp directory: $ROCKPROP_DIR"
