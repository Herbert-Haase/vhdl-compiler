#!/usr/bin/env bash
set -euo pipefail

ANTLR4_VERSION="4.13.1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
GENERATED_DIR="${BUILD_DIR}/generated"
RUNTIME_DIR="${BUILD_DIR}/antlr4-runtime"
ANTLR4_JAR="${BUILD_DIR}/antlr-${ANTLR4_VERSION}-complete.jar"
ANTLR4_JAR_URL="https://repo1.maven.org/maven2/org/antlr/antlr4/${ANTLR4_VERSION}/antlr4-${ANTLR4_VERSION}-complete.jar"
GRAMMAR_FILE="${SCRIPT_DIR}/grammar/VHDL.g4"
RUNTIME_LIB="${RUNTIME_DIR}/lib/libantlr4-runtime.a"
OUT="${BUILD_DIR}/vhdl_lexer"

die() { echo "error: $*" >&2; exit 1; }
need() { command -v "$1" &>/dev/null || die "'$1' not found in PATH"; }

need java; need git; need g++; need curl

mkdir -p "${BUILD_DIR}" "${GENERATED_DIR}"

# ANTLR4 JAR
if [[ ! -f "${ANTLR4_JAR}" ]]; then
    echo ">> Downloading ANTLR4 ${ANTLR4_VERSION} JAR..."
    curl -fsSL --tlsv1.2 -o "${ANTLR4_JAR}" "${ANTLR4_JAR_URL}" \
        || die "Failed to download ANTLR4 JAR"
fi

# ANTLR4 C++ runtime
if [[ ! -f "${RUNTIME_LIB}" ]]; then
    echo ">> Cloning ANTLR4 runtime..."
    RUNTIME_SRC="${BUILD_DIR}/antlr4-src"
    [[ ! -d "${RUNTIME_SRC}" ]] && \
        git clone --depth 1 --branch "${ANTLR4_VERSION}" \
            https://github.com/antlr/antlr4.git "${RUNTIME_SRC}"

    cmake -S "${RUNTIME_SRC}/runtime/Cpp" -B "${RUNTIME_DIR}" \
          -DCMAKE_BUILD_TYPE=Release \
          -DANTLR_BUILD_CPP_TESTS=OFF \
          -DWITH_DEMO=OFF \
          -DCMAKE_INSTALL_PREFIX="${RUNTIME_DIR}"
    cmake --build "${RUNTIME_DIR}" --parallel "$(nproc)"
    cmake --install "${RUNTIME_DIR}"
fi

RUNTIME_INCLUDE="${RUNTIME_DIR}/include/antlr4-runtime"
[[ ! -d "${RUNTIME_INCLUDE}" ]] && \
    RUNTIME_INCLUDE="${BUILD_DIR}/antlr4-src/runtime/Cpp/runtime/src"

# Generate lexer only
GENERATED_LEXER="${GENERATED_DIR}/VHDLLexer.cpp"

if [[ ! -f "${GENERATED_LEXER}" || "${GRAMMAR_FILE}" -nt "${GENERATED_LEXER}" ]]; then
    echo ">> Generating lexer from ${GRAMMAR_FILE}..."
    java -jar "${ANTLR4_JAR}" \
        -Dlanguage=Cpp \
        -no-listener \
        -no-visitor \
        -o "${GENERATED_DIR}" \
        "${GRAMMAR_FILE}" \
        || die "ANTLR4 lexer generation failed"
fi

# Compile
echo ">> Compiling vhdl_lexer..."
g++ -std=c++17 -O2 \
    -o "${OUT}" \
    "${SCRIPT_DIR}/src/main.cpp" \
    "${GENERATED_LEXER}" \
    -I"${GENERATED_DIR}" \
    -I"${RUNTIME_INCLUDE}" \
    "${RUNTIME_DIR}/lib/libantlr4-runtime.a" \
    || die "Compilation failed"

echo ">> Built: ${OUT}"
