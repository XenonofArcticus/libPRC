macro( _addLibrary TRGTNAME )
    if( BUILD_SHARED_LIBS )
        add_library( ${TRGTNAME} SHARED ${ARGN} )
    else()
        add_library( ${TRGTNAME} STATIC ${ARGN} )
    endif()

    include_directories(
        ${PROJECT_SOURCE_DIR}/src/libPRC
        ${ZLIB_INCLUDE_DIR}
    )
    add_definitions( -DPRC_LIBRARY )

    target_link_libraries( ${TRGTNAME}
        ${ZLIB_LIBRARY}
    )

    set_target_properties( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Plugin ${TRGTNAME}" )
endmacro()

macro( _addOSGPlugin TRGTNAME )
    if( BUILD_SHARED_LIBS )
        add_library( ${TRGTNAME} MODULE ${ARGN} )
    else()
        add_library( ${TRGTNAME} STATIC ${ARGN} )
    endif()

    include_directories(
        ${PROJECT_SOURCE_DIR}/src/libPRC
        ${OPENSCENEGRAPH_INCLUDE_DIRS}
    )

    target_link_libraries( ${TRGTNAME}
        ${OPENSCENEGRAPH_LIBRARIES}
        libPRC
        ${ZLIB_LIBRARY}
    )

    set_target_properties( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Plugin ${TRGTNAME}" )
endmacro()
