# if (CMAKE_VERSION VERSION_LESS 3.2)
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
# else()
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
# endif()

include(DownloadProject)
download_project(   PROJ                libnest2d
                    GIT_REPOSITORY      https://github.com/tamasmeszaros/libnest2d.git
                    GIT_TAG             f33c1078e7427471ac6546676b9639e38600c20c
                    SOURCE_DIR          ${CMAKE_SOURCE_DIR}/xs/src/libnest2d
                    CONFIGURE_COMMAND   ""
)