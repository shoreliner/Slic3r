# if (CMAKE_VERSION VERSION_LESS 3.2)
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
# else()
#     set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
# endif()

include(DownloadProject)
download_project(   PROJ                binpack2d
                    GIT_REPOSITORY      https://github.com/tamasmeszaros/binpack2d.git
                    GIT_TAG             833baf5f4e05a55ed9eb5b2ce39b597d8ed5d929
                    SOURCE_DIR          ${CMAKE_SOURCE_DIR}/xs/src/binpack2d
)