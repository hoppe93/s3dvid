
# Basic compiler/linker configuration
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/Modules/" CACHE PATH "CMake module path" FORCE)

include_directories("${PROJECT_SOURCE_DIR}/src/include")

# Set libraries to use
option(DEBUG "Compile with debug symbols and no optimzations" OFF)

set(main
	#"${PROJECT_SOURCE_DIR}/src/axes.c"
	"${PROJECT_SOURCE_DIR}/src/camera.c"
	"${PROJECT_SOURCE_DIR}/src/main.c"
	"${PROJECT_SOURCE_DIR}/src/png.c"
	"${PROJECT_SOURCE_DIR}/src/s3d.c"
)

if (DEBUG)
	message(STATUS "Compiling in DEBUG mode")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -fopenmp -O0 -g -pg -D_FILE_OFFSET_BITS=64")
else (DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -fopenmp -O3 -march=native -D_FILE_OFFSET_BITS=64")
endif (DEBUG)

add_executable(s3dvid ${main})
target_link_libraries(s3dvid m)

# Compile with MATLAB support
find_package(Matlab COMPONENTS MAT_LIBRARY MX_LIBRARY)
if (Matlab_FOUND)
    include_directories(${Matlab_INCLUDE_DIRS})
    message(${Matlab_MAT_LIBRARY})
    message(${Matlab_MX_LIBRARY})
    target_link_libraries(s3dvid ${Matlab_MAT_LIBRARY} ${Matlab_MX_LIBRARY})
else (Matlab_FOUND)
    message(FATAL_ERROR "MATLAB support was requested, but no MATLAB installation was found")
endif (Matlab_FOUND)

# Find OpenMP!
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENMP_C_FLAGS}")
endif (OPENMP_FOUND)

# Find libpng
find_package(PNG REQUIRED)
if (PNG_FOUND)
	add_definitions(${PNG_DEFINITIONS})
	include_directories(${PNG_INCLUDE_DIRS})
	target_link_libraries(s3dvid ${PNG_LIBRARIES})
endif (PNG_FOUND)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/include/config.h.in"
	"${PROJECT_SOURCE_DIR}/src/include/config.h"
)

