cmake_minimum_required(VERSION 3.10)
project(os_overview VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Network REQUIRED)

set(SOURCES
    src/main.cpp
    src/server.cpp
    src/systeminfo.cpp
    src/filemanager.cpp
    src/usermanager.cpp
    src/servicemanager.cpp
    src/networkdiscovery.cpp
    src/processmanager.cpp
)

set(HEADERS
    src/server.h
    src/systeminfo.h
    src/filemanager.h
    src/usermanager.h
    src/servicemanager.h
    src/networkdiscovery.h
    src/processmanager.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
install(FILES ${CMAKE_SOURCE_DIR}/os-overview.service DESTINATION /lib/systemd/system)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "OS Overview Server")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5network5")
include(CPack)
