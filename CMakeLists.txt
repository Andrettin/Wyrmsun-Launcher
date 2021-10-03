#     ___
#    / _/    ___
#   / /   ——  \ \ ,—,__ _____ ____ ___  _____  __———,——____
#  | (    )(   \ / / \ || ___)  _ ' _ \(  ___)/ / \ ||  __ \
#  |  \__/  \__/ \ \_) ||(   | ( | | ) )\__ \( (__) || /  ) )
#   \_____/\____/ \_  / |_\  /_| /_|/_/\_____)\___/\_\_\ /_/
#                 /__/

cmake_minimum_required(VERSION 3.7.0)

project(wyrmsun_launcher VERSION 1.0.0)

include_directories(
	src
	${CMAKE_CURRENT_BINARY_DIR}
)

set(wyrmsun_launcher_SRCS
	src/main.cpp
	src/launcher.rc
)

set(wyrmsun_launcher_HDRS
	src/process_manager.h
)

set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo" CACHE STRING "" FORCE)

#Qt5 modules
#different modules have different licenses, make sure all modules used here are compatible with the LGPL
set(CMAKE_AUTOMOC ON)
find_package(Qt5 5.12 COMPONENTS Core REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)
find_package(Qt5 5.12 COMPONENTS Gui REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)
find_package(Qt5 5.12 COMPONENTS Widgets REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)
find_package(Qt5 5.12 COMPONENTS Multimedia REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)
find_package(Qt5 5.12 COMPONENTS Qml REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)
find_package(Qt5 5.12 COMPONENTS Quick REQUIRED) #licensed under the GPL 2.0 (as well as the LGPL 3.0)

set(QT_LIBRARIES
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::Multimedia
	Qt5::Qml
	Qt5::Quick
)

set(wyrmsun_launcher_LIBS
	${QT_LIBRARIES}
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 AND MSVC)
	#enable parallel compilation
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Compile launcher

add_executable(wyrmsun_launcher WIN32 ${wyrmsun_launcher_SRCS} ${wyrmsun_launcher_HDRS})

if (MSVC)
	target_compile_options(wyrmsun_launcher PRIVATE /W4 /w44800 /wd4458)
endif()

target_link_libraries(wyrmsun_launcher ${wyrmsun_launcher_LIBS})

set_target_properties(wyrmsun_launcher PROPERTIES OUTPUT_NAME "launcher")

########### clean files ###############

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")
