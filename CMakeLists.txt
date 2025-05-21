cmake_minimum_required(VERSION 3.19)
project(scu_net_auto_login VERSION 1.2.1 LANGUAGES C CXX)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

message("Include path: ${CMAKE_SOURCE_DIR}/thirdparty/mini-gmp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 设置mini-gmp.c为C语言文件
set_source_files_properties(thirdparty/mini-gmp/mini-gmp.c PROPERTIES LANGUAGE C)

add_library(mini-gmp STATIC
    thirdparty/mini-gmp/mini-gmp.c
)

# 对mini-gmp库设置C语言标准 - 现在移动到add_library之后
set_target_properties(mini-gmp PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

target_include_directories(mini-gmp PUBLIC
    ${CMAKE_SOURCE_DIR}/thirdparty/mini-gmp
)

qt_add_executable(scu_net_auto_login
    src/main.cpp
    src/Loginer.h src/Loginer.cpp
    src/User.h src/User.cpp
    src/RSAUtils.h src/RSAUtils.cpp
    src/PlatformUtils.h src/PlatformUtils.cpp
    src/Global.h src/Global.cpp
    src/Settings.h src/Settings.cpp
    src/AutoTickDevice.h src/AutoTickDevice.cpp
    src/SCUNetLoginApplication.h src/SCUNetLoginApplication.cpp
)

target_link_libraries(scu_net_auto_login
    PRIVATE
        mini-gmp
        Qt::Core
        Qt::Network
)

set_target_properties(scu_net_auto_login PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
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
