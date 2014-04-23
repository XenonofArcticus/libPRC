
#ifndef __PRC_EXPORT_H__
#define __PRC_EXPORT_H__ 1


#if defined( _MSC_VER ) || defined( __CYGWIN__ ) || defined( __MINGW32__ ) || defined( __BCPLUSPLUS__ ) || defined( __MWERKS__ )
    #if defined( PRC_STATIC )
        #define PRC_EXPORT
    #elif defined( PRC_LIBRARY )
        #define PRC_EXPORT __declspec( dllexport )
    #else
        #define PRC_EXPORT __declspec( dllimport )
    #endif
#else
    #define PRC_EXPORT
#endif


// __PRC_EXPORT_H__
#endif
