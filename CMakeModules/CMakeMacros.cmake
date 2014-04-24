macro( _addLibrary TRGTNAME )
    # Check to see if we are forcing a static library.
    set( _optionsPlusFiles ${ARGN} )
    list( GET _optionsPlusFiles 0 _option )
    if( _option STREQUAL "FORCE_STATIC" )
        # Remove the FORCE_STATIC option, leaving only file names.
        list( REMOVE_AT _optionsPlusFiles 0 )
    endif()

    if( BUILD_SHARED_LIBS AND NOT ( _option STREQUAL "FORCE_STATIC" ) )
        add_library( ${TRGTNAME} SHARED ${_optionsPlusFiles} )
    else()
        add_library( ${TRGTNAME} STATIC ${_optionsPlusFiles} )
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
        ${PRC_INCLUDE_DIR}
        ${OPENSCENEGRAPH_INCLUDE_DIRS}
    )

    target_link_libraries( ${TRGTNAME}
        ${OPENSCENEGRAPH_LIBRARIES}
        ${PRC_LIBRARY}
        ${ZLIB_LIBRARY}
    )

    set_target_properties( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Plugin ${TRGTNAME}" )
endmacro()
