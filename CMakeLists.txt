# redshift GUI CMake build file
cmake_minimum_required(VERSION 2.6)
project("Redshift GUI")
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
elseif(WIN32)
	option(ENABLE_WINGDI "Enable win32 GDI at compile time" true)
	option(ENABLE_WINGUI "Enable win32 GUI at compile time" true)
endif(UNIX)

# Source files
file(GLOB RSGSRCFiles "${RSG_SRC_DIR}/*.c" "${RSG_SRC_DIR}/*.h")
set(RSGSRC
	${RSGSRCFiles}
	)
if(UNIX)
	if(ENABLE_RANDR)
		set(RSGSRC ${RSGSRC}
			${RSG_SRC_DIR}/backends/randr.c
			${RSG_SRC_DIR}/backends/randr.h)
	endif(ENABLE_RANDR)
	if(ENABLE_VIDMODE)
		set(RSGSRC ${RSGSRC}
			${RSG_SRC_DIR}/backends/vidmode.c
			${RSG_SRC_DIR}/backends/vidmode.h)
	endif(ENABLE_VIDMODE)
	if(ENABLE_GTK)
		set(RSGSRC ${RSGSRC}
			${RSG_SRC_DIR}/gui/gui.c
			${RSG_SRC_DIR}/gui/gui.h)
	endif(ENABLE_GTK)
elseif(WIN32)
	if(ENABLE_WINGDI)
		set(RSGSRC ${RSGSRC}
			${RSG_SRC_DIR}/backends/w32gdi.c
			${RSG_SRC_DIR}/backends/w32gdi.h)
	endif(ENABLE_WINGDI)
	if(ENABLE_WINGUI)
		set(RSGSRC ${RSGSRC}
			${RSG_SRC_DIR}/gui/iupgui.c
			${RSG_SRC_DIR}/gui/gui.h)
	endif(ENABLE_WINGUI)
endif(UNIX)
add_executable(RSGBIN WIN32 ${RSGSRC})
include_directories(${RSG_SRC_DIR})

# 3rd party packages
unset(RSG_INCLUDES)
unset(RSG_LIBS)
if(UNIX)
	if(ENABLE_GTK)
		cmake_policy(SET CMP0011 OLD)
		cmake_policy(PUSH)
		find_package(GTK2 2.10 REQUIRED gtk)
		cmake_policy(POP)
	endif(ENABLE_GTK)
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
	if(ENABLE_WINGUI)
		find_package(IUP COMPONENTS controls)
		set(RSG_INCLUDES ${RSG_INCLUDES}
			${IUP_INCLUDE_DIR}
			)
		set(RSG_LIBS ${RSG_LIBS}
			${IUP_LIBRARIES}
			)
	endif(ENABLE_WINGUI)
endif(UNIX)
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
	set(CMAKE_C_FLAGS "-std=gnu99" CACHE STRING "C flags" FORCE)
endif(UNIX)

# Preprocessor definitions
unset(RSG_DEFS)
set(RSG_DEFS ${RSG_DEFS}
	-DPACKAGE="redshiftgui"
	-DPACKAGE_BUGREPORT="https://bugs.launchpad.net/redshift"
	)
CHECK_INCLUDE_FILE(libintl.h ENABLE_NLS)
CHECK_INCLUDE_FILE(sys/signal.h HAVE_SYS_SIGNAL_H)
#if(ENABLE_NLS)
#	set(RSG_DEFS ${RSG_DEFS} -DENABLE_NLS)
#endif(ENABLE_NLS)
if(HAVE_SYS_SIGNAL_H)
	set(RSG_DEFS ${RSG_DEFS} -DHAVE_SYS_SIGNAL_H)
endif(HAVE_SYS_SIGNAL_H)
if(UNIX)
	if(ENABLE_RANDR)
		set(RSG_DEFS ${RSG_DEFS} -DENABLE_RANDR)
	endif(ENABLE_RANDR)
	if(ENABLE_VIDMODE)
		set(RSG_DEFS ${RSG_DEFS} -DENABLE_VIDMODE)
	endif(ENABLE_VIDMODE)
	if(ENABLE_GTK)
		set(RSG_DEFS ${RSG_DEFS} -DENABLE_GTK)
	endif(ENABLE_GTK)
else(WIN32)
	if(ENABLE_WINGDI)
		set(RSG_DEFS ${RSG_DEFS} -DENABLE_WINGDI)
	endif(ENABLE_WINGDI)
	if(ENABLE_WINGUI)
		set(RSG_DEFS ${RSG_DEFS} -DENABLE_WINGUI)
	endif(ENABLE_WINGUI)
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

