
set(sources
    Authentications.cpp
    Authentications.hpp
    Address.hpp
    Main.cpp
    NamedMutex.cpp
    NamedMutex.hpp
    Settings.cpp
    Settings.hpp
    Worker.cpp
    Worker.hpp
)

add_executable(gchsd
    ${sources}
)

target_link_libraries(gchsd PRIVATE
    libnstd::Socket
    ZLIB::ZLIB
)

source_group("" FILES ${sources})
set_property(TARGET gchsd PROPERTY FOLDER "src")


set(sources
    AskPass.cpp
)

add_executable(gchsd-askpass
    ${sources}
)
target_link_libraries(gchsd-askpass PRIVATE
    libnstd::Core
)

source_group("" FILES ${sources})
set_property(TARGET gchsd-askpass PROPERTY FOLDER "src")



install(TARGETS gchsd gchsd-askpass DESTINATION usr/sbin)

