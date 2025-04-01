cmake_minimum_required(VERSION 3.19)
project(scu_net_auto_login LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup()

qt_add_executable(scu_net_auto_login
    main.cpp
    BigInt.hpp
    Loginer.h Loginer.cpp
    RSAUtils.h RSAUtils.cpp
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
