get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
file(GLOB_RECURSE SOURCES ${CURRENT_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${CURRENT_DIR}/*.h)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
