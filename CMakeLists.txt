# redshift GUI CMake build file
cmake_minimum_required(VERSION 2.6)
project("Redshift GUI")

# Appends ${ARGN} to ${_var} if ${_val} is defined
macro(APPEND_IF_VAR _var _val)
	if(${_val})
		set(${_var} ${${_var}} ${ARGN})
	endif(${_val})
endmacro(APPEND_IF_VAR _var _val)

set(APP_NAME "redshiftgui")

# Project version
set(RSG_VERSION_MAJOR 0)
set(RSG_VERSION_MINOR 1)
set(RSG_VERSION_PATCH 0)
set(RSG_VERSION
	"${RSG_VERSION_MAJOR}.${RSG_VERSION_MINOR}.${RSG_VERSION_PATCH}")

# Project directories
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
set(RSG_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
message(STATUS "${ARCH}")
set(RSG_OUT_DIR "${PROJECT_BINARY_DIR}" CACHE STRING "Output dir")

# CMake modules to include
include(CheckIncludeFile)

# Configuration options
if(UNIX)
	option(ENABLE_RANDR "Enable Xrandr at compile time" true)
	option(ENABLE_VIDMODE "Enable Vidmode at compile time" true)
	option(ENABLE_GTK "Enable GTK GUI at compile time" true)
	option(ENABLE_IUP "Enable IUP GUI at compile time" false)
elseif(WIN32)
	option(ENABLE_WINGDI "Enable win32 GDI at compile time" true)
	option(ENABLE_GTK "Enable GTK GUI at compile time" false)
	option(ENABLE_IUP "Enable IUP GUI at compile time" true)
endif(UNIX)

if( ENABLE_GTK AND ENABLE_IUP )
	message(FATAL_ERROR "Cannot have both GTK and IUP enabled")
endif( ENABLE_GTK AND ENABLE_IUP )

# Source files
set(RSGSRC
	${RSG_SRC_DIR}/argparser.h
	${RSG_SRC_DIR}/argparser.c
	${RSG_SRC_DIR}/common.h
	${RSG_SRC_DIR}/gamma.h
	${RSG_SRC_DIR}/gamma.c
	${RSG_SRC_DIR}/logger.h
	${RSG_SRC_DIR}/logger.c
	${RSG_SRC_DIR}/options.h
	${RSG_SRC_DIR}/options.c
	${RSG_SRC_DIR}/redshiftgui.c
	${RSG_SRC_DIR}/solar.h
	${RSG_SRC_DIR}/solar.c
	${RSG_SRC_DIR}/systemtime.h
	${RSG_SRC_DIR}/systemtime.c
	)
APPEND_IF_VAR(RSGSRC ENABLE_GTK
		${RSG_SRC_DIR}/gui/gtkgui.c
		${RSG_SRC_DIR}/gui/gtkgui.h)
APPEND_IF_VAR(RSGSRC ENABLE_IUP
		${RSG_SRC_DIR}/gui/iupgui.c
		${RSG_SRC_DIR}/gui/iupgui.h)
if(UNIX)
	APPEND_IF_VAR(RSGSRC ENABLE_RANDR
			${RSG_SRC_DIR}/backends/randr.c
			${RSG_SRC_DIR}/backends/randr.h)
	APPEND_IF_VAR(RSGSRC ENABLE_VIDMODE
			${RSG_SRC_DIR}/backends/vidmode.c
			${RSG_SRC_DIR}/backends/vidmode.h)
elseif(WIN32)
	APPEND_IF_VAR(RSGSRC ENABLE_WINGDI
			${RSG_SRC_DIR}/backends/w32gdi.c
			${RSG_SRC_DIR}/backends/w32gdi.h)
endif(UNIX)
add_executable(RSGBIN WIN32 ${RSGSRC})
include_directories(${RSG_SRC_DIR})

# 3rd party packages
unset(RSG_INCLUDES CACHE)
unset(RSG_LIBS CACHE)
if(ENABLE_GTK)
	cmake_policy(SET CMP0011 OLD)
	cmake_policy(PUSH)
	find_package(GTK2 2.10 REQUIRED gtk)
	cmake_policy(POP)
elseif(ENABLE_IUP)
	find_package(IUP)
	set(RSG_INCLUDES ${RSG_INCLUDES}
		${IUP_INCLUDE_DIR}
		)
	set(RSG_LIBS ${RSG_LIBS}
		${IUP_LIBRARIES}
		)
endif(ENABLE_GTK)
if(UNIX)
	find_package(X11)
	if(ENABLE_RANDR)
		find_package(XCB COMPONENTS randr)
	endif(ENABLE_RANDR)
	if(ENABLE_VIDMODE)
		find_package(XLIB COMPONENTS xf86vm)
	endif(ENABLE_VIDMODE)
	set(RSG_INCLUDES ${RSG_INCLUDES}
		${GTK2_INCLUDE_DIRS}
		${X11_INCLUDE_DIR}
		${XCB_INCLUDE_DIR}
		${XLIB_INCLUDE_DIR}
		)
	set(RSG_LIBS ${RSG_LIBS}
		${GTK2_LIBRARIES}
		${X11_LIBRARIES}
		${XCB_LIBRARIES}
		${XLIB_LIBRARIES}
		)
elseif(WIN32)
	if(MSVC)
		set(RSG_INCLUDES ${RSG_INCLUDES} "${PROJECT_SOURCE_DIR}/msvc")
	endif(MSVC)
endif(UNIX)
find_package(CURL)
set(RSG_INCLUDES ${RSG_INCLUDES} ${CURL_INCLUDE_DIRS})
set(RSG_LIBS ${RSG_LIBS} ${CURL_LIBRARIES})

set(RSG_INCLUDE_DIRS ${RSG_INCLUDES} CACHE STRING "Redshift GUI includes")
set(RSG_LIBRARIES ${RSG_LIBS} CACHE STRING "Redshift GUI libs")
foreach(_inc ${RSG_INCLUDES})
	message(STATUS "   Include: ${_inc}")
endforeach(_inc ${RSG_INCLUDES})
foreach(_lib ${RSG_LIBS})
	message(STATUS "   Lib: ${_lib}")
endforeach(_lib ${RSG_LIBS})

# Default to debug build
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
# Compiler flags
if(UNIX)
	set(CMAKE_C_FLAGS "-Wall -Werror" CACHE STRING "C flags" FORCE)
endif(UNIX)

# Preprocessor definitions
unset(RSG_DEFS CACHE)
set(RSG_DEFS ${RSG_DEFS}
	-DPACKAGE="redshiftgui"
	-DPACKAGE_BUGREPORT="https://bugs.launchpad.net/redshift"
	)
CHECK_INCLUDE_FILE(libintl.h ENABLE_NLS)
CHECK_INCLUDE_FILE(sys/signal.h HAVE_SYS_SIGNAL_H)
APPEND_IF_VAR(RSG_DEFS ENABLE_NLS -DENABLE_NLS)
APPEND_IF_VAR(RSG_DEFS HAVE_SYS_SIGNAL_H -DHAVE_SYS_SIGNAL_H)
APPEND_IF_VAR(RSG_DEFS ENABLE_GTK -DENABLE_GTK)
APPEND_IF_VAR(RSG_DEFS ENABLE_IUP -DENABLE_IUP)
if(UNIX)
	APPEND_IF_VAR(RSG_DEFS ENABLE_RANDR -DENABLE_RANDR)
	APPEND_IF_VAR(RSG_DEFS ENABLE_VIDMODE -DENABLE_VIDMODE)
else(WIN32)
	APPEND_IF_VAR(RSG_DEFS ENABLE_WINGDI -DENABLE_WINGDI)
endif(UNIX)
set(RSG_DEFINITIONS ${RSG_DEFS} CACHE STRING "Redshift GUI defines")
add_definitions(${RSG_DEFINITIONS})
foreach(_def ${RSG_DEFS})
	message(STATUS "   Definition: ${_def}")
endforeach(_def ${RSG_DEFS})

# Includes and libraries
include_directories(${RSG_INCLUDE_DIRS})
target_link_libraries(RSGBIN ${RSG_LIBRARIES})
set_target_properties(RSGBIN PROPERTIES
	OUTPUT_NAME					${APP_NAME}
	OUTPUT_NAME_DEBUG			${APP_NAME}_debug
	RUNTIME_OUTPUT_DIRECTORY	${RSG_OUT_DIR})

