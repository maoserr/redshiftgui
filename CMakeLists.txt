# redshift GUI CMake build file
cmake_minimum_required(VERSION 2.6)
project("Redshift GUI")

set(APP_NAME "redshiftgui")
set(PRO_NAME "RedshiftGUI")

# Project version
set(RSG_VERSION_MAJOR 0)
set(RSG_VERSION_MINOR 2)
set(RSG_VERSION_PATCH 4)
set(RSG_VERSION
	"${RSG_VERSION_MAJOR}.${RSG_VERSION_MINOR}.${RSG_VERSION_PATCH}")
# Project information
set(RSG_COPYRIGHT "GPL")
set(RSG_AUTHOR "Mao Yu")
set(RSG_CONTACT "maoserr@gmail.com")
set(RSG_HOMEPAGE "http://maoserr.github.io/projects/redshiftgui/")
set(RSG_BUGREPORT "http://github.com/maoserr/redshiftgui/issues")
set(RSG_DESC "Monitor color temperature adjustment tool")


# Appends ${ARGN} to ${_var} if ${_val} is defined
macro(APPEND_IF_VAR _var _val)
	if(${_val})
		set(${_var} ${${_var}} ${ARGN})
	endif(${_val})
endmacro(APPEND_IF_VAR _var _val)

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
	option(ENABLE_GTK "Enable GTK GUI at compile time" false)
	option(ENABLE_IUP "Enable IUP GUI at compile time" true)
	option(PACKAGE_DEB "Package deb files" false)
elseif(WIN32)
	option(ENABLE_WINGDI "Enable win32 GDI at compile time" true)
	option(ENABLE_WINGUI "Enable Windows GUI at compile time" true)
endif(UNIX)

if( ENABLE_GTK AND ENABLE_IUP )
	message(FATAL_ERROR "Cannot have both GTK and IUP enabled")
elseif( ENABLE_GTK )
	message(FATAL_ERROR "GTK GUI is not implemented")
endif( ENABLE_GTK AND ENABLE_IUP )

# Nonproject Source files
set(RSGNSRC
	${RSG_SRC_DIR}/thirdparty/argparser.h
	${RSG_SRC_DIR}/thirdparty/argparser.c
	${RSG_SRC_DIR}/thirdparty/logger.h
	${RSG_SRC_DIR}/thirdparty/logger.c
	${RSG_SRC_DIR}/thirdparty/stb_image.h
	${RSG_SRC_DIR}/thirdparty/stb_image.c
	${RSG_SRC_DIR}/common.h
	${RSG_SRC_DIR}/gamma.h
	${RSG_SRC_DIR}/location.h
	${RSG_SRC_DIR}/options.h
	${RSG_SRC_DIR}/solar.h
	${RSG_SRC_DIR}/systemtime.h
	)
# Project Source files
set(RSGSRC
	${RSG_SRC_DIR}/gamma.c
	${RSG_SRC_DIR}/location.c
	${RSG_SRC_DIR}/netutils.c
	${RSG_SRC_DIR}/options.c
	${RSG_SRC_DIR}/redshiftgui.c
	${RSG_SRC_DIR}/solar.c
	${RSG_SRC_DIR}/systemtime.c
	${RSG_SRC_DIR}/resources/redshift.c
	${RSG_SRC_DIR}/resources/redshift-idle.c
	${RSG_SRC_DIR}/resources/sun.c
	${RSG_SRC_DIR}/resources/sunback.c
	)
APPEND_IF_VAR(RSGSRC ENABLE_GTK
		${RSG_SRC_DIR}/gui/gtkgui.c)
APPEND_IF_VAR(RSGNSRC ENABLE_GTK
		${RSG_SRC_DIR}/gui/gtkgui.h)
APPEND_IF_VAR(RSGSRC ENABLE_IUP
		${RSG_SRC_DIR}/gui/iupgui.c
		${RSG_SRC_DIR}/gui/iupgui_gamma.c
		${RSG_SRC_DIR}/gui/iupgui_location.c
		${RSG_SRC_DIR}/gui/iupgui_main.c
		${RSG_SRC_DIR}/gui/iupgui_settings.c
		)
APPEND_IF_VAR(RSGNSRC ENABLE_IUP
		${RSG_SRC_DIR}/gui/iupgui.h
		${RSG_SRC_DIR}/gui/iupgui_gamma.h
		${RSG_SRC_DIR}/gui/iupgui_location.h
		${RSG_SRC_DIR}/gui/iupgui_main.h
		${RSG_SRC_DIR}/gui/iupgui_settings.h
		)
APPEND_IF_VAR(RSGSRC ENABLE_WINGUI
		${RSG_SRC_DIR}/gui/win32gui.c
		${RSG_SRC_DIR}/gui/win32gui_gamma.c
		)
APPEND_IF_VAR(RSGNSRC ENABLE_WINGUI
		${RSG_SRC_DIR}/gui/win32gui.h
		${RSG_SRC_DIR}/gui/win32gui_gamma.h
		)
if(UNIX AND NOT APPLE)
	APPEND_IF_VAR(RSGSRC ENABLE_RANDR
			${RSG_SRC_DIR}/backends/randr.c)
	APPEND_IF_VAR(RSGNSRC ENABLE_RANDR
			${RSG_SRC_DIR}/backends/randr.h)
	APPEND_IF_VAR(RSGSRC ENABLE_VIDMODE
			${RSG_SRC_DIR}/backends/vidmode.c)
	APPEND_IF_VAR(RSGNSRC ENABLE_VIDMODE
			${RSG_SRC_DIR}/backends/vidmode.h)
elseif(WIN32)
	APPEND_IF_VAR(RSGSRC ENABLE_WINGDI
			${RSG_SRC_DIR}/backends/w32gdi.c)
	APPEND_IF_VAR(RSGNSRC ENABLE_WINGDI
			${RSG_SRC_DIR}/backends/w32gdi.h)
	# Resource file
	configure_file(${RSG_SRC_DIR}/win32/app.rc.in
		${RSG_SRC_DIR}/win32/app.rc)
	configure_file(${RSG_SRC_DIR}/win32/app.manifest.in
		${RSG_SRC_DIR}/win32/app.manifest)
	configure_file(${RSG_SRC_DIR}/win32/app.64.manifest.in
		${RSG_SRC_DIR}/win32/app.64.manifest)
	include(AddMingwRC)
	ADD_RC_FILE(${RSG_SRC_DIR}/win32 app RSGNSRC)
endif(UNIX AND NOT APPLE)

# 3rd party packages
unset(RSG_INCLUDE_DIRS CACHE)
unset(RSG_LIBRARIES CACHE)
if(ENABLE_GTK)
	cmake_policy(SET CMP0011 OLD)
	cmake_policy(PUSH)
	find_package(GTK2 2.10 REQUIRED gtk)
	cmake_policy(POP)
elseif(ENABLE_IUP)
	if(UNIX)
		cmake_policy(SET CMP0011 OLD)
		cmake_policy(PUSH)
		find_package(GTK2 2.10 REQUIRED gtk)
		cmake_policy(POP)
	endif(UNIX)

	set(IUP_FIND_STATIC TRUE)
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
		m
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
if(WIN32)
	set(RSG_LIBS ${RSG_LIBS} comctl32.lib wsock32.lib ws2_32.lib Wldap32.lib)
endif(WIN32)

# Remove duplicate entries
list(REMOVE_DUPLICATES RSG_INCLUDES)
list(REMOVE_DUPLICATES RSG_LIBS)
set(RSG_INCLUDE_DIRS ${RSG_INCLUDES} CACHE STRING "Redshift GUI includes")
set(RSG_LIBRARIES ${RSG_LIBS} CACHE STRING "Redshift GUI libs")
foreach(_inc ${RSG_INCLUDES})
	message(STATUS "   Include: ${_inc}")
endforeach(_inc ${RSG_INCLUDES})
foreach(_lib ${RSG_LIBS})
	message(STATUS "   Lib: ${_lib}")
endforeach(_lib ${RSG_LIBS})

# Default to release build
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
# Compiler flags
if(UNIX)
	set(CMAKE_C_FLAGS "-Wall" CACHE STRING "C flags" FORCE)
elseif(MSVC)
	set(CMAKE_C_FLAGS_DEBUG "/W3 /D_DEBUG /MTd /Zi /Ob0 /Od /RTC1" CACHE STRING
		"Debug flags" FORCE)
	set(CMAKE_C_FLAGS_RELEASE "/W3 /MT /O2 /Ob2 /D NDEBUG" CACHE STRING
		"Release flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "/MANIFEST:NO /STACK:10000000 /machine:X86" CACHE
		STRING "Linker flags" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE" CACHE
		STRING "Debug Linker flags" FORCE)
endif(UNIX)

# Preprocessor definitions
unset(RSG_DEFINITIONS CACHE)
set(RSG_DEFS ${RSG_DEFS}
	PACKAGE="${APP_NAME}"
	PACKAGE_HOME="${RSG_HOMEPAGE}"
	PACKAGE_BUGREPORT="${RSG_BUGREPORT}"
	PACKAGE_VER="${RSG_VERSION}"
	)
CHECK_INCLUDE_FILE(libintl.h ENABLE_NLS)
CHECK_INCLUDE_FILE(sys/signal.h HAVE_SYS_SIGNAL_H)
#APPEND_IF_VAR(RSG_DEFS ENABLE_NLS ENABLE_NLS)
APPEND_IF_VAR(RSG_DEFS HAVE_SYS_SIGNAL_H HAVE_SYS_SIGNAL_H)
APPEND_IF_VAR(RSG_DEFS ENABLE_GTK ENABLE_GTK)
APPEND_IF_VAR(RSG_DEFS ENABLE_IUP ENABLE_IUP)
if(UNIX)
	APPEND_IF_VAR(RSG_DEFS ENABLE_RANDR ENABLE_RANDR)
	APPEND_IF_VAR(RSG_DEFS ENABLE_VIDMODE ENABLE_VIDMODE)
else(WIN32)
	APPEND_IF_VAR(RSG_DEFS ENABLE_WINGDI ENABLE_WINGDI)
endif(UNIX)
foreach(_def ${RSG_DEFS})
	set(RSG_DEFINITIONS ${RSG_DEFINITIONS} -D${_def})
endforeach(_def ${RSG_DEFS})
set(RSG_DEFINITIONS ${RSG_DEFINITIONS} CACHE STRING "Redshift GUI defines")
add_definitions(${RSG_DEFINITIONS})
foreach(_def ${RSG_DEFS})
	message(STATUS "   Definition: ${_def}")
endforeach(_def ${RSG_DEFS})

# Includes and libraries
include_directories(${RSG_INCLUDE_DIRS})
include_directories(${RSG_SRC_DIR})
add_executable(RSGBIN WIN32 ${RSGSRC} ${RSGNSRC})
target_link_libraries(RSGBIN ${RSG_LIBRARIES})
set_target_properties(RSGBIN PROPERTIES
	OUTPUT_NAME					${APP_NAME}
	OUTPUT_NAME_DEBUG			${APP_NAME}_debug
	RUNTIME_OUTPUT_DIRECTORY	${RSG_OUT_DIR})

# Documentation
find_package(Doxygen COMPONENTS DOXYGEN_SKIP_DOT)
if(DOXYGEN_FOUND)
	configure_file(${PROJECT_SOURCE_DIR}/docs/Doxyfile.in
		${RSG_SRC_DIR}/Doxyfile)
	add_custom_target(docs ${DOXYGEN_EXECUTABLE}
		WORKING_DIRECTORY ${RSG_SRC_DIR})
endif(DOXYGEN_FOUND)

# Splint target
find_program(SPLINT_PATH splint)
if(SPLINT_PATH)
	foreach(loop_var ${RSG_INCLUDES})
		set(SPLINT_PARAMS ${SPLINT_PARAMS} -I${loop_var})
	endforeach(loop_var ${RSG_INCLUDES})
	set(SPLINT_PARAMS ${SPLINT_PARAMS}
		-I${RSG_SRC_DIR}
		)
	if(UNIX)
		set(SPLINT_PARAMS ${SPLINT_PARAMS}
			-systemdirs "/usr/include:/usr/include/sys")
	endif(UNIX)
	foreach(loop_var ${RSG_DEFS})
		set(SPLINT_PARAMS ${SPLINT_PARAMS} -D${loop_var})
	endforeach(loop_var ${RSG_DEFS})
	if(UNIX)
		set(SPLINT_PARAMS ${SPLINT_PARAMS} 
			+posixlib
			)
	elseif(MSVC)
		set(SPLINT_PARAMS ${SPLINT_PARAMS}
			-D__STDC__=1
			-D_WIN32
			-D_MSC_VER=1500
			-D_M_IX86=400
			-DNOGDI
			-DWIN32_LEAN_AND_MEAN
			-D__int64=long
			-D__w64=
			-D__ptr64=
			-D__forceinline=inline
			)
	endif(UNIX)
	set(SPLINT_PARAMS ${SPLINT_PARAMS}
		+show-scan
		+boolint
		-mts ${PROJECT_SOURCE_DIR}/splint)
	set(SPLINT_PARAMS ${SPLINT_PARAMS}
		${RSGSRC}
		)
	add_custom_target(splint ${SPLINT_PATH} ${SPLINT_PARAMS}
		VERBATIM)
endif(SPLINT_PATH)

# Installation rules
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(CPACK_PACKAGE_NAME ${PRO_NAME})
set(CPACK_PACKAGE_VENDOR ${RSG_AUTHOR})
set(CPACK_PACKAGE_CONTACT "${RSG_AUTHOR} [${RSG_CONTACT}]")
set(CPACK_PACKAGE_VERSION_MAJOR ${RSG_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${RSG_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${RSG_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME
	"${PRO_NAME}-${RSG_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
	"${RSG_DESC}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PRO_NAME})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.txt")

if(UNIX)
	install(TARGETS RSGBIN
		DESTINATION bin
		CONFIGURATIONS Release)
	install(DIRECTORY 
		${PROJECT_SOURCE_DIR}/data/applications
		${PROJECT_SOURCE_DIR}/data/icons
		DESTINATION share
		CONFIGURATIONS Release)
	set(CPACK_GENERATOR TGZ)
	if(PACKAGE_DEB)
		set(CPACK_GENERATOR ${CPACK_GENERATOR} DEB)
	endif(PACKAGE_DEB)
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${RSG_AUTHOR} <${RSG_CONTACT}>")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Monitor color adjustment tool.")
	set(CPACK_DEBIAN_PACKAGE_SECTION "util")
	set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
		"Monitor color adjustment tool. Sets your monitor color temperature based on sun elevation.")
	if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
		set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
	endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl3,libxcb-randr0")
	# Arch linux package rules
	add_custom_target(arch_pack
		${PROJECT_SOURCE_DIR}/data/aur_pack.sh ${RSG_VERSION}
		${PROJECT_SOURCE_DIR}/buildLinuxi686/${PRO_NAME}-${RSG_VERSION}-${CMAKE_SYSTEM_NAME}-i686.tar.gz
		${PROJECT_SOURCE_DIR}/buildLinuxx86_64/${PRO_NAME}-${RSG_VERSION}-${CMAKE_SYSTEM_NAME}-x86_64.tar.gz
		PKGBUILD
		)
elseif(WIN32)
	install(TARGETS RSGBIN
		DESTINATION .
		CONFIGURATIONS Release)
	set(CPACK_GENERATOR NSIS ZIP)
	set(CPACK_PACKAGE_EXECUTABLES "" "")
	set(CPACK_NSIS_MENU_LINKS
		"${APP_NAME}.exe" "${PRO_NAME}"
		"http://www.mao-yu.com" "Mao Yu's Homepage")
	set(CPACK_NSIS_WELCOME_BITMAP "${PROJECT_SOURCE_DIR}/data\\\\NSIS_Welcome.bmp")
endif(UNIX)

INCLUDE(CPack)
