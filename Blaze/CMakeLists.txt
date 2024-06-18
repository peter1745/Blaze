cmake_minimum_required(VERSION 3.28)

project(Blaze)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Main.cpp
    HTML/Parser.cpp
    HTML/DOM/Document.cpp
    HTML/DOM/Node.cpp)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            CSTM)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ../Blaze/
        ../Libraries/CSTM/)
