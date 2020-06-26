cmake_minimum_required(VERSION 3.5)


get_filename_component(projectName InferDT NAME)

message("cmake for ${projectName}")

file(
	GLOB_RECURSE
	source_files
	src/*
)

set(externLibs
        minisat
)

############ OPTIONS DE COMPILATION ##########################
add_definitions(-DNDEBUG)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O3 -static")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -g")
##############################################################


if(TARGET ${projectName})

	message("${projectName} is already defined")

else()

	add_executable (
		${projectName}
		${source_files}
	)

	install (TARGETS ${projectName} DESTINATION bin)

        set(LIB_LINK
		${externLibs}
	)

	target_link_libraries(${projectName}
	    ${LIB_LINK}
	)

endif()




