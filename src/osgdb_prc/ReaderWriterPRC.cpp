
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>

#include "OSG2PRC.h"

#ifdef PRC_USE_ASYMPTOTE
#  include <oPRCFile.h>
#else
#  include <libPRC.h>
#  include <PRCFile.h>
#endif


class ReaderWriterPRC : public osgDB::ReaderWriter
{
public:
    ReaderWriterPRC()
    {
        supportsExtension( "prc", "Adobe PRC (Product Representation Conpact)" );
    }

    WriteResult writeNode( const osg::Node& node, const std::string& fileName, const Options* opt=NULL ) const
    {
        std::ofstream ostr( fileName.c_str(),
            std::ios_base::out | std::ios_base::binary );
        if( ostr.bad() )
            return( "Failed to open std::ofstream for " + fileName );

        WriteResult writeResult( writeNode( node, ostr, opt ) );
        ostr.close();

        return( writeResult );
    }
    WriteResult writeNode( const osg::Node& node, std::ostream& ostr, const Options* opt=NULL ) const
    {
#ifdef PRC_USE_ASYMPTOTE
        oPRCFile* prcFile( new oPRCFile( ostr ) );
        if( prcFile == NULL )
            return( "NULL prcFile." );

        osg::Node* nonConstNode( const_cast< osg::Node* >( &node ) );
        OSG2PRC osg2prc( prcFile );
        nonConstNode->accept( osg2prc );

        if( !( prcFile->finish() ) )
            return( "prcFile::finish() returned false." );

        return( osgDB::ReaderWriter::WriteResult::FILE_SAVED );
#else
        prc::File* prcFile( prc::open( fileName.c_str(), "w" ) );
        if( prcFile == NULL )
            return( "prc::file() returned NULL." );

        osg::Node* nonConstNode( const_cast< osg::Node* >( &node ) );
        OSG2PRC osg2prc;
        nonConstNode->accept( osg2prc );

        if( !( prc::close( prcFile ) ) )
            return( "prc::close() returned false." );

        return( osgDB::ReaderWriter::WriteResult::FILE_SAVED );
#endif
    }
};


REGISTER_OSGPLUGIN( prc, ReaderWriterPRC );
