cmake_minimum_required (VERSION 3.10)
project (QCTP C)

# --[ Output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---[ Includes
set(QCTP_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
set(QCTP_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(QCTP_THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party)
include_directories(${QCTP_SRC_INCLUDE_DIR} ${QCTP_TEST_INCLUDE_DIR} ${QCTP_THIRD_PARTY_INCLUDE_DIR})

# ---[ flex && bison
# FIND_PACKAGE(BISON REQUIRED)
# SET(BisonOutput ${CMAKE_SOURCE_DIR}/src/parser/bison.c)
# IF(BISON_FOUND)
#   ADD_CUSTOM_COMMAND(
#     OUTPUT ${BisonOutput}
#     COMMAND ${BISON_EXECUTABLE} 
#     --output=${BisonOutput}
#     ${CMAKE_SOURCE_DIR}/src/parser/parser.y
#     COMMENT "Generating parser.c"
#     )
# ENDIF()

# FIND_PACKAGE(FLEX REQUIRED)
# SET(FlexOutput ${CMAKE_SOURCE_DIR}/src/parser/lexer.c)
# IF(FLEX_FOUND)
#   ADD_CUSTOM_COMMAND(
#     OUTPUT ${FlexOutput}
#     COMMAND ${FLEX_EXECUTABLE}
#     --outfile=${FlexOutput}
#     ${CMAKE_SOURCE_DIR}/src/parser/lexer.l
#     COMMENT "Generating lexer.c"
#     )
# ENDIF()
# ADD_LIBRARY(MyLib ${BisonOutput} ${FlexOutput})

find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*/*.c)

add_executable(qctp ${PROJECT_SOURCE_DIR}/src/main.c ${srcs})

target_link_libraries(qctp ssl)
