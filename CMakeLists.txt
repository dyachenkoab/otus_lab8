cmake_minimum_required(VERSION 3.16)

project(bayan VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(PROJECT_VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.67 COMPONENTS program_options regex filesystem REQUIRED)

add_executable(${PROJECT_NAME}
   src/main.cpp
   src/strategy.h
   src/strategy.cpp
   src/handler.h
   src/handler.cpp
   src/processor.h
   src/processor.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -pedantic
    -Werror
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} )

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
