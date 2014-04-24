
set( LIB_PUBLIC_HEADERS
    Export.h
    libPRC.h
    PRCFile.h
    PRCNode.h
)

_addLibrary( libPRC
    libPRC.cpp
    PRCFile.cpp
    PRCNode.cpp
    ${LIB_PUBLIC_HEADERS}
)
