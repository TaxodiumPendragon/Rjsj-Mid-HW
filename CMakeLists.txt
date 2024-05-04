cmake_minimum_required(VERSION 3.14)
project(docman)

add_executable(docman main.cpp utils.hpp citation.h)
target_include_directories(docman PRIVATE 
    ${CMAKE_SOURCE_DIR}/third_parties/nlohmann
    ${CMAKE_SOURCE_DIR}/third_parties/cpp-httplib
)

set_target_properties(docman PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)

# 对于 Windows，链接到 ws2_32
if(WIN32)
    target_link_libraries(docman ws2_32)
endif()
