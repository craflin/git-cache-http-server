
set(sources
    Address.hpp
    HttpRequest.cpp
    HttpRequest.hpp
    Main.cpp
    Settings.cpp
    Settings.hpp
    Worker.cpp
    Worker.hpp
)

add_executable(gchsd
    ${sources}
)

target_link_libraries(gchsd PRIVATE
    libnstd::Socket CURL::libcurl
)

source_group("" FILES ${sources})

set_property(TARGET gchsd PROPERTY FOLDER "src")

install(TARGETS gchsd DESTINATION usr/sbin)

