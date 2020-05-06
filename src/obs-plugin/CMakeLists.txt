project(obs-plugin)

find_package(Libobs REQUIRED)

find_library(AVFOUNDATION AVFoundation)
find_library(COCOA Cocoa)
find_library(COREFOUNDATION CoreFoundation)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COCOA Cocoa)
find_library(COREMEDIAIO CoreMediaIO)
find_library(IOSURFACE IOSurface)
find_library(IOKIT IOKit)

set(CMAKE_PREFIX_PATH "${QTDIR}")
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${AVFOUNDATION}
                    ${COCOA}
                    ${COREFOUNDATION}
                    ${COREMEDIA}
                    ${COREVIDEO}
                    ${COREMEDIAIO}
                    ${COCOA}
                    ${IOSURFACE}
                    ${LIBOBS_INCLUDE_DIR}
                    "${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
                    ${Qt5Core_INCLUDES}
                    ${Qt5Widgets_INCLUDES}
                    ./)

set(obs-plugin_SOURCES
	plugin-main.cpp)

add_library(obs-plugin MODULE
	${obs-plugin_SOURCES}
	${obs-plugin_HEADERS})

target_link_libraries(obs-plugin
	${AVFOUNDATION}
	${COCOA}
	${COREFOUNDATION}
	${COREMEDIA}
	${COREVIDEO}
	${COREMEDIAIO}
	${IOSURFACE}
	${IOKIT}
	${LIBOBS_LIB}
	${OBS_FRONTEND_LIB}
	Qt5::Core
	Qt5::Widgets)

set_target_properties(obs-plugin PROPERTIES
	PREFIX ""
	OUTPUT_NAME "obs-mac-virtualcam")
