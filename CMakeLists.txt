cmake_minimum_required(VERSION 3.5)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "compiler: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "ws_sepa build type: " ${CMAKE_BUILD_TYPE})


project(ws_sepa CXX C)

configure_file(${PROJECT_SOURCE_DIR}/config_app.h.in
${PROJECT_SOURCE_DIR}/config_app.h @ONLY)

if (CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} _type) 
    if (_type STREQUAL release)
        message(STATUS "build type RELEASE") 
        set(CMAKE_BUILD_DIR build_release)
        add_compile_options(-D_RELEASE)
        #add flto
        add_compile_options(${GCC_LTO})
        #add_compile_options(-O2)
        add_definitions(${GCC_LTO})
    else ()
        set(CMAKE_BUILD_DIR build)
        message(STATUS "build type DEBUG")
        add_compile_options(-D_DEBUG)
        add_compile_options(-g)
    endif ()
else (CMAKE_BUILD_TYPE)
    message(STATUS "build type DEBUG")
    set(CMAKE_BUILD_DIR build)
    add_compile_options(-D_DEBUG)
    add_compile_options(-g)
endif (CMAKE_BUILD_TYPE)

add_compile_options("-DPLUTOBOOK_BUILD_STATIC")
add_compile_options("-Wno-sign-compare")
add_compile_options("-Wno-unused-function")

    message("Building standalone tool")

set(base_sources
    main.cpp
    config_profile.cpp
    util.cpp
    logger.cpp
    webservicesapi.cpp
    sepaxml.cpp
    tinyxml2.cpp
    wsfiledescriptor.cpp
)
    set(prj_sources
        ${base_sources}
    )

set(private_headers

)



#RapidXML
include_directories("third_party/RapidXML/RapidXML")
include_directories("third_party/rapidXPath-cpp/src")

#rapidjson - conflicts with plutobook document.h
include_directories("third_party/rapidjson/include")
#xml2json
include_directories("third_party/xml2json/include")


add_compile_options(-DCPPHTTPLIB_OPENSSL_SUPPORT)
add_executable(${PROJECT_NAME} ${prj_sources})
find_package(OpenSSL)
if(OpenSSL_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENSSL)
endif()

