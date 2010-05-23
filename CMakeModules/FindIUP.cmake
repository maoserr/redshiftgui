# Package finder for IUP libraries (from tecgraf)
# - Be sure to set the COMPONENTS to the components you want to link to.
# - The IUP_LIBRARIES variable is set ONLY to your COMPONENTS list
# - To use only a specific component
#	check the IUP_LIBRARIES_${COMPONENT} variable

# Define search directories depending on system
if(CMAKE_COMPILER_IS_GNUCC)
	set(_search_path_inc ENV CPATH)
	set(_search_path_lib ENV LIBRARY_PATH)
endif(CMAKE_COMPILER_IS_GNUCC)

find_path(IUP_INCLUDE_DIR iup.h
	HINTS ${_search_path_inc})
find_library(IUP_MAIN_LIB iup
	HINTS ${_search_path_lib})

set(IUP_LIBRARIES ${IUP_MAIN_LIB})
foreach(COMPONENT ${IUP_FIND_COMPONENTS})
	find_library(IUP_LIBRARIES_${COMPONENT} iup${COMPONENT}
		HINTS ${_search_path_lib})
	set(IUP_LIBRARIES ${IUP_LIBRARIES}
		${IUP_LIBRARIES_${COMPONENT}})
	mark_as_advanced(IUP_LIBRARIES_${COMPONENT})
endforeach(COMPONENT ${IUP_FIND_COMPONENTS})

if(WIN32)
	set(IUP_LIBRARIES ${IUP_LIBRARIES} comctl32)
endif(WIN32)

mark_as_advanced(IUP_INCLUDE_DIR)
mark_as_advanced(IUP_MAIN_LIB)
set(IUP_LIBRARIES ${IUP_LIBRARIES} CACHE INTERNAL
	"IUP Libraries" FORCE)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IUP DEFAULT_MSG
	IUP_LIBRARIES IUP_INCLUDE_DIR)
