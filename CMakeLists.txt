cmake_minimum_required(VERSION 3.22)
project(FydelisDiskSuite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── ATIVA O AUTOMOC DO QT ─────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    main.cpp
    mainwindow.cpp
    modules/dashboard.cpp
    modules/diskformatter.cpp
    modules/partitionrecovery.cpp
    modules/filerecovery.cpp
    modules/diskcloner.cpp
    modules/diskinfo.cpp
    modules/usbcreator.cpp
    utils/diskmanager.cpp
)

set(HEADERS
    mainwindow.h
    modules/dashboard.h
    modules/diskformatter.h
    modules/partitionrecovery.h
    modules/filerecovery.h
    modules/diskcloner.h
    modules/diskinfo.h
    modules/usbcreator.h
    utils/diskmanager.h
    utils/styles.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
