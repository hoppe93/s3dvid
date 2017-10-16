
# Basic compiler/linker configuration
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/Modules/" CACHE PATH "CMake module path" FORCE)

include_directories("${PROJECT_SOURCE_DIR}/src/include")

# Set libraries to use
option(DEBUG "Compile with debug symbols and no optimzations" OFF)
option(USE_HDF5 "Link in support for HDF5" OFF)
option(USE_MATLAB "Link in support for MATLAB *.mat files" ON)

set(main
	"${PROJECT_SOURCE_DIR}/src/axes.c"
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

# Compile with HDF5 support?
if (USE_HDF5)
	find_package(HDF5 REQUIRED)
	if (HDF5_FOUND)
		include_directories(${HDF5_INCLUDE_DIRS})
		message(STATUS "Path to HDF5: ${HDF5_INCLUDE_DIRS}")
		target_link_libraries(s3dvid ${HDF5_LIBRARIES})
	else (HDF5_FOUND)
		message(FATAL_ERROR "HDF5 support was requested, but no HDF5 library was found on this system")
	endif (HDF5_FOUND)
endif (USE_HDF5)

# Compile with MATLAB support?
if (USE_MATLAB)
	find_package(Matlab COMPONENTS MAT_LIBRARY MX_LIBRARY)
	if (Matlab_FOUND)
		include_directories(${Matlab_INCLUDE_DIRS})
		message(${Matlab_MAT_LIBRARY})
		message(${Matlab_MX_LIBRARY})
		target_link_libraries(s3dvid ${Matlab_MAT_LIBRARY} ${Matlab_MX_LIBRARY})
	else (Matlab_FOUND)
		message(FATAL_ERROR "MATLAB support was requested, but no MATLAB installation was found")
	endif (Matlab_FOUND)
endif (USE_MATLAB)

# Find OpenMP!
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENMP_C_FLAGS}")
endif (OPENMP_FOUND)

# Find MPI
if (USE_MPI)
	find_package(MPI)
	if (MPI_C_FOUND)
		include_directories(${MPI_C_INCLUDE_PATH})

		set_target_properties(s3dvid PROPERTIES COMPILE_FLAGS "${MPI_C_COMPILE_FLAGS}")
		set_target_properties(s3dvid PROPERTIES LINK_FLAGS "${MPI_C_LINK_FLAGS}")
		target_link_libraries(s3dvid "${MPI_C_LIBRARIES}")
	else (MPI_C_FOUND)
		message(FATAL_ERROR "MPI support was requested, but no MPI library was found")
	endif (MPI_C_FOUND)
endif (USE_MPI)

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

