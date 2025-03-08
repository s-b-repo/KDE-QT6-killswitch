### File: CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(KDE-QT6-killswitch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Add the executable from the source files
add_executable(KDE-QT6-killswitch src/main.cpp)

target_link_libraries(KDE-QT6-killswitch PRIVATE Qt6::Widgets Qt6::Network)

# Install the executable and resources
install(TARGETS LinuxKillSwitch DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/KDE-QT6-killswitch/resources)
install(FILES polkit/50.killswitch.rules DESTINATION share/polkit-1/rules.d)
