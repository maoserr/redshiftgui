if (WIN32)
	if(MINGW)
		set(CMAKE_RC_COMPILER "windres")
	endif(MINGW)
endif(WIN32)

# Parameters
# - rc_path			Path to the rc file
# - rc_file			Name of the rc file
# - src_variable	A variable that the rc file will be appended to.
macro(ADD_RC_FILE rc_path rc_file src_variable)
if (WIN32)
	if(MINGW)
		# resource compilation for mingw
		ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${rc_file}.o
							COMMAND ${CMAKE_RC_COMPILER}
								-i${rc_path}/${rc_file}.rc
								-o ${CMAKE_CURRENT_BINARY_DIR}/${rc_file}.o)
		set(${src_variable} ${${src_variable}}
			${CMAKE_CURRENT_BINARY_DIR}/${rc_file}.o)
	else(MINGW)
		set (${src_variable} ${${src_variable}}
			${rc_path}/${rc_file}.rc)
  endif(MINGW)
endif (WIN32)
endmacro(ADD_RC_FILE rc_path rc_file src_variable)

