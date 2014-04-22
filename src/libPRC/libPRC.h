
#ifndef __lib_prc_h__
#define __lib_prc_h__ 1


namespace prc {


class File;


/** \brief Open a PRC file.
\details If \c readOnly is false, create the file if it
doesn't already exist. This is the default behavior.

Returns NULL if the file can not be opened.

Note: PRC reading isn't currently supported, so passing
true for \c readOnly currently displays a warning to std::cerr,
then behaves as if readOnly==false. */
File* open( const char* fileName, const bool readOnly=false );

/** \brief Close a PRC file.
\detauls Returns true on success. Returns false if the file
can not be closed, or if \c prcFile is not a pointer to a
valid prc::File object. */
bool clode( File* preFile );


// namespace prc
}


// __lib_prc_h__
#endif
