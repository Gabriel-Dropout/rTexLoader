# Find all CPP files in the examples directory
file(GLOB EXAMPLE_SOURCES "*.cpp")

# Iterate over each CPP file and create an executable
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    # Get the base name of the file (without extension)
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)

    # Create the executable for this example
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})

    # Link against the library
    target_link_libraries(${EXAMPLE_NAME} PRIVATE raylib)
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
endforeach()

# Copy the resource folder to the binary folder
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})