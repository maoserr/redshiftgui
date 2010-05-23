# Package finder for XLIB libraries
#  - Be sure to set the COMPONENTS to the components you want to link to
#  - The XLIB_LIBRARIES variable is set ONLY to your COMPONENTS list
#  - To use only a specific component
#	check the XLIB_LIBRARIES_${COMPONENT} variable

# Define search directories depending on system
if(CMAKE_COMPILER_IS_GNUCC)
	set(_search_path_inc ENV CPATH)
	set(_search_path_lib ENV LIBRARY_PATH)
endif(CMAKE_COMPILER_IS_GNUCC)

find_path(XLIB_INCLUDE_DIR X11/Xlib.h
	HINTS ${_search_path_inc})

unset(XLIB_LIBRARIES)
foreach(COMPONENT ${XLIB_FIND_COMPONENTS})
	find_library(XLIB_LIBRARIES_${COMPONENT} X${COMPONENT}
		HINTS ${_earch_path_lib})
	set(XLIB_LIBRARIES ${XLIB_LIBRARIES}
		${XLIB_LIBRARIES_${COMPONENT}})
	mark_as_advanced(XLIB_LIBRARIES_${COMPONENT})
endforeach(COMPONENT ${XLIB_FIND_COMPONENTS})

mark_as_advanced(XLIB_INCLUDE_DIR)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XLIB DEFAULT_MSG
	XLIB_LIBRARIES XLIB_INCLUDE_DIR)

