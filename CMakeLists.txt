cmake_minimum_required(VERSION 3.20)
project(vhdl_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
find_package(Java REQUIRED COMPONENTS Runtime)

set(ANTLR4_VERSION 4.13.1)
set(ANTLR4_JAR "${CMAKE_BINARY_DIR}/antlr-${ANTLR4_VERSION}-complete.jar")
set(ANTLR4_JAR_URL
    "https://repo1.maven.org/maven2/org/antlr/antlr4/${ANTLR4_VERSION}/antlr4-${ANTLR4_VERSION}-complete.jar"
)

if(NOT EXISTS "${ANTLR4_JAR}")
    file(
        DOWNLOAD "${ANTLR4_JAR_URL}" "${ANTLR4_JAR}"
        STATUS ANTLR4_JAR_DOWNLOAD_STATUS
        TLS_VERIFY ON
        SHOW_PROGRESS
    )
    list(GET ANTLR4_JAR_DOWNLOAD_STATUS 0 ANTLR4_JAR_DOWNLOAD_RESULT)
    if(NOT ANTLR4_JAR_DOWNLOAD_RESULT EQUAL 0)
        list(GET ANTLR4_JAR_DOWNLOAD_STATUS 1 ANTLR4_JAR_DOWNLOAD_MESSAGE)
        message(FATAL_ERROR "Failed to download ANTLR jar: ${ANTLR4_JAR_DOWNLOAD_MESSAGE}")
    endif()
endif()

FetchContent_Declare(
    antlr4_runtime
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG ${ANTLR4_VERSION}
    SOURCE_SUBDIR runtime/Cpp
)
set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_DEMO OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(antlr4_runtime)

set(GRAMMAR_FILE "${CMAKE_CURRENT_SOURCE_DIR}/grammar/VHDL.g4")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_SOURCES
    "${GENERATED_DIR}/VHDLLexer.cpp"
    "${GENERATED_DIR}/VHDLParser.cpp"
    "${GENERATED_DIR}/VHDLBaseListener.cpp"
    "${GENERATED_DIR}/VHDLListener.cpp"
)

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND ${Java_JAVA_EXECUTABLE} -jar "${ANTLR4_JAR}" -Dlanguage=Cpp -listener -no-visitor -o "${GENERATED_DIR}" "${GRAMMAR_FILE}"
    DEPENDS "${GRAMMAR_FILE}"
    COMMENT "Generating C++ parser sources from ${GRAMMAR_FILE}"
    VERBATIM
)

add_custom_target(generate_vhdl_parser DEPENDS ${GENERATED_SOURCES})

add_executable(
    vhdl_compiler
    src/main.cpp
    ${GENERATED_SOURCES}
)
add_dependencies(vhdl_compiler generate_vhdl_parser)
target_include_directories(vhdl_compiler PRIVATE "${GENERATED_DIR}")
target_include_directories(vhdl_compiler PRIVATE "${antlr4_runtime_SOURCE_DIR}/runtime/Cpp/runtime/src")
target_link_libraries(vhdl_compiler PRIVATE antlr4_static)

enable_testing()
add_test(
    NAME parse_minimal_entity
    COMMAND vhdl_compiler "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/minimal_entity.vhd"
)
add_test(
    NAME reject_mismatched_entity_name
    COMMAND vhdl_compiler "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/mismatched_entity.vhd"
)
set_tests_properties(reject_mismatched_entity_name PROPERTIES WILL_FAIL TRUE)
