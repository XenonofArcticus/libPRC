
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <libPRC.h>
#include <PRCFile.h>

#include "OSG2PRC.h"


class ReaderWriterPRC : public osgDB::ReaderWriter
{
public:
    ReaderWriterPRC()
    {
        supportsExtension( "prc", "Adobe PRC (Product Representation Conpact)" );
    }

    WriteResult writeNode( const osg::Node& node, const std::string& fileName, const Options* opt=NULL ) const
    {
        prc::File* prcFile( prc::open( fileName.c_str(), "w" ) );
        if( prcFile == NULL )
            return( "prc::file() returned NULL." );

        osg::Node* nonConstNode( const_cast< osg::Node* >( &node ) );
        OSG2PRC osg2prc;
        nonConstNode->accept( osg2prc );

        if( !( prc::close( prcFile ) ) )
            return( "prc::close() returned false." );

        return( osgDB::ReaderWriter::WriteResult::FILE_SAVED );
    }
};


REGISTER_OSGPLUGIN( prc, ReaderWriterPRC );
