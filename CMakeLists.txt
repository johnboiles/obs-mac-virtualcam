cmake_minimum_required(VERSION 3.17)

project(obs-mac-virtualcam VERSION 1.1.3)
set(PLUGIN_AUTHOR "John Boiles")
set(MACOS_BUNDLEID "com.johnboiles.obs-mac-virtualcam")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11")

configure_file(
    installer/installer-macOS.pkgproj.in
    ../installer/installer-macOS.generated.pkgproj
)

configure_file(
    ci/ci_includes.sh.in
    ../ci/ci_includes.generated.sh
)
configure_file(
    ci/ci_includes.cmd.in
    ../ci/ci_includes.generated.cmd
)

add_subdirectory(src/obs-plugin)
add_subdirectory(src/dal-plugin)
