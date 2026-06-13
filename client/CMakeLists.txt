cmake_minimum_required(VERSION 3.16)
project(BankApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(BankApp
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    BankClient.cpp
    BankClient.hpp
)

target_link_libraries(BankApp PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)