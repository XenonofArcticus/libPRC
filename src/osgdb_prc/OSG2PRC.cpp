
#include "osg2prc.h"

#include <osg/Transform>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Geometry>

#include <iostream>


OSG2PRC::OSG2PRC()
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

#ifdef PRC_USE_ASYMPTOTE
OSG2PRC::OSG2PRC(oPRCFile* prcFile)
	: osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
	_prcFile( prcFile )
{
}
#endif

OSG2PRC::~OSG2PRC()
{
}


void OSG2PRC::apply( osg::Node& node )
{
    std::cout << "Found osg::Node" << std::endl;

    processNewNode( node.getName() );

    if( node.getStateSet() != NULL )
        apply( node.getStateSet() );

    traverse( node );

	finishNode();
}
void OSG2PRC::apply( osg::Transform& trans )
{
    std::cout << "Found osg::Transform" << std::endl;

    osg::Matrix m;
    trans.computeLocalToWorldMatrix( m, NULL );
    processTransformNode( trans.getName(), m );

    if( trans.getStateSet() != NULL )
        apply( trans.getStateSet() );

    traverse( trans );

	finishNode();
}
void OSG2PRC::apply( osg::Geode& geode )
{
    std::cout << "Found osg::Geode" << std::endl;

    processNewNode( geode.getName() );

    if( geode.getStateSet() != NULL )
        apply( geode.getStateSet() );

    for( unsigned int idx=0; idx < geode.getNumDrawables(); ++idx )
    {
        const osg::Geometry* geom( geode.getDrawable( idx )->asGeometry() );
        if( geom != NULL )
            apply( geom );
    }

    traverse( geode );
}

void OSG2PRC::apply( const osg::StateSet* stateSet )
{
    std::cout << "Found osg::StateSet" << std::endl;

    const osg::StateAttribute* sa( stateSet->getAttribute( osg::StateAttribute::MATERIAL ) );
    if( sa != NULL )
    {
        const osg::Material* mat( static_cast< const osg::Material* >( sa ) );
        std::cout << "TBD: Add material to PRC" << std::endl;
    }
}
void OSG2PRC::apply( const osg::Geometry* geom )
{
    std::cout << "Found osg::Geometry" << std::endl;

    const osg::Array* array( geom->getVertexArray() );
    if( array->getType() != osg::Array::Vec3ArrayType )
    {
        std::cerr << "Unsupported array type." << std::endl;
        return;
    }
    const osg::Vec3Array* vertices( static_cast< const osg::Vec3Array* >( array ) );
    std::cout << "TBD: Add vertex array to PRC, size " << array->getNumElements() << std::endl;

    array = geom->getNormalArray();
    if( array != NULL )
    {
        if( array->getType() != osg::Array::Vec3ArrayType )
        {
            std::cerr << "Unsupported array type." << std::endl;
            return;
        }
        const osg::Vec3Array* normals( static_cast< const osg::Vec3Array* >( array ) );
        std::cout << "TBD: Add normals array to PRC, size " << array->getNumElements() << std::endl;
    }

    for( unsigned int idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
    {
        const osg::PrimitiveSet* ps( geom->getPrimitiveSet( idx ) );
        // TBD DrawArrays, DrawElementsUInt, etc.
        std::cout << "TBD: Add PrimitiveSet to PRC" << std::endl;
    }
}

void OSG2PRC::processNewNode( const std::string& name )
{
	std::cout << "Adding new node (with name " << name << ") to PRC" << std::endl;

	_prcFile->begingroup( name.c_str() );
}
void OSG2PRC::processTransformNode( const std::string& name, const osg::Matrix& matrix )
{
    std::cout << "TBD: Add matrix to PRC" << std::endl;

	//_prcFile->begingroup( name.c_str(), NULL,  (const double *)matrix._mat);
}
void OSG2PRC::finishNode()
{
	_prcFile->endgroup();
}