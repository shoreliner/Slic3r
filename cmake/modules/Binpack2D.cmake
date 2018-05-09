# if (CMAKE_VERSION VERSION_LESS 3.2)
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
# else()
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
# endif()

include(DownloadProject)
download_project(   PROJ                binpack2d
                    GIT_REPOSITORY      https://github.com/tamasmeszaros/binpack2d.git
                    GIT_TAG             f8b9487cfd701fc5c1f9aefd6cbf4359cd65bd67
                    SOURCE_DIR          ${CMAKE_SOURCE_DIR}/xs/src/binpack2d
)