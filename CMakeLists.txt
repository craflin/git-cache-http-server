
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0048 NEW)

project(gchsd VERSION 0.2.0)

set(CDEPLOY_NO_DEBUG_BUILD True)
set(CDEPLOY_NO_COMPILER True)

find_package(CURL REQUIRED)
if(NOT TARGET CURL::libcurl)
    add_library(CURL::libcurl INTERFACE IMPORTED)
    set_target_properties(CURL::libcurl
        PROPERTIES 
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${CURL_LIBRARIES}"
    )
endif()

include(CDeploy)

add_subdirectory(3rdparty)
add_subdirectory(src)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Colin Graf <colin.graf@sovereign-labs.com>")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A mirroring Git HTTP server daemon")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
#set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/craflin/gchsd")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.15),libcurl4")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")

include(CPack)

install(FILES etc/gchsd.conf DESTINATION etc)
install(FILES etc/default/gchsd DESTINATION etc/default)
install(PROGRAMS etc/init.d/gchsd DESTINATION etc/init.d)
