add_library(binpack2d_bridge STATIC
    # Binpack2DBridge.cpp
    # Binpack2DBridge.hpp
    ModelExtensions.hpp
    ModelExtensions.cpp
)

target_include_directories(binpack2d_bridge PUBLIC BEFORE ${BINPACK2D_HEADERS})

if(BINPACK2D_UNITTESTS)
   # If we want the binpack2d unit tests we need to build and executable with
   # all the libslic3r dependencies

   add_library(libslic3r_standalone STATIC ${LIBDIR}/libslic3r/utils.cpp)
   set(BINPACK2D_TEST_LIBRARIES
       libslic3r libslic3r_standalone nowide libslic3r_gui admesh miniz
       ${Boost_LIBRARIES} clipper ${EXPAT_LIBRARIES} ${GLEW_LIBRARIES}
       polypartition poly2tri ${TBB_LIBRARIES} ${wxWidgets_LIBRARIES}
       ${CURL_LIBRARIES}
       PARENT_SCOPE
   )
endif()
