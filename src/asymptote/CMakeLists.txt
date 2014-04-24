
set( LIB_PUBLIC_HEADERS
    PRC.h
    PRCbitStream.h
    PRCdouble.h
    oPRCFile.h
    writePRC.h
)

_addLibrary( asymptote FORCE_STATIC
    PRCbitStream.cc
    PRCdouble.cc
    oPRCFile.cc
    writePRC.cc
    ${LIB_PUBLIC_HEADERS}
)
