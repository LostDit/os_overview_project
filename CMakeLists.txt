cmake_minimum_required(VERSION 3.10)
project(OS_Overview_and_Client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Network Widgets REQUIRED) 

add_subdirectory(os_overview)
add_subdirectory(client)
