cmake_minimum_required(VERSION 3.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS
    ON
)

project(ats)

add_subdirectory(third_party)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src ats_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/api/src api_src)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/module module_src)

add_executable(
    ${PROJECT_NAME}
    ${ats_src} ${api_src} ${module_src}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/api/include/
        ${CMAKE_CURRENT_LIST_DIR}/module/include/
        ${CMAKE_CURRENT_LIST_DIR}/include/
        ${CMAKE_CURRENT_LIST_DIR}/utils/
        ${CMAKE_CURRENT_LIST_DIR}/third_party/bounds_checking_function/include/
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE bounds_checking_function
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/
)
