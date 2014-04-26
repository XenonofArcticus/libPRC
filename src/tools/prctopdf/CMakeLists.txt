# Make sure the compiler can find include files from our Haru library. 
include_directories (${LIBHARU_SOURCE_DIR}/LibHaru) 

# Make sure the linker can find the Haru library once it is built. 
link_directories (${LIBHARU_BINARY_DIR}/LibHaru) 

add_executable (prctopdf
	prctopdf.cpp ) 

# Link the executable to the LibHaru library. 
target_link_libraries (prctopdf LibHaru)