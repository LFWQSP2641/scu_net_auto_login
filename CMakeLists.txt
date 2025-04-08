cmake_minimum_required(VERSION 3.19)
project(scu_net_auto_login VERSION 1.1.2 LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

qt_add_executable(scu_net_auto_login
    src/main.cpp
    src/BigInt.hpp
    src/Loginer.h src/Loginer.cpp
    src/RSAUtils.h src/RSAUtils.cpp
    src/PlatformUtils.h src/PlatformUtils.cpp
    src/Global.h src/Global.cpp
    src/Settings.h src/Settings.cpp
    src/AutoTickDevice.h src/AutoTickDevice.cpp
    src/SCUNetLoginApplication.h src/SCUNetLoginApplication.cpp
)

target_link_libraries(scu_net_auto_login
    PRIVATE
        Qt::Core
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS scu_net_auto_login
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET scu_net_auto_login
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
