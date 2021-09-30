#!/bin/bash

CC=${1}
XXD=${2}
INPUT_FILE=${3}
OUTPUT_DIR=${4}
OUTPUT_FILENAME=${5}

BC_FILENAME="$(basename ${INPUT_FILE}).bc"

cd ${OUTPUT_DIR}
${CC} -Os -emit-llvm -c "${INPUT_FILE}" -o "${BC_FILENAME}"
${XXD} -i "${BC_FILENAME}" > "${OUTPUT_FILENAME}"
