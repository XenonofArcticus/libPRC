
#include "osg2prc.h"

#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Geometry>

#include <iostream>


OSG2PRC::OSG2PRC()
    : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}
OSG2PRC::~OSG2PRC()
{
}


void OSG2PRC::apply( osg::Node& node )
{
    std::cout << "Node" << std::endl;

    if( node.getStateSet() != NULL )
        apply( node.getStateSet() );

    traverse( node );
}
void OSG2PRC::apply( osg::MatrixTransform& mt )
{
    std::cout << "MatrixTransform" << std::endl;

    if( mt.getStateSet() != NULL )
        apply( mt.getStateSet() );

    traverse( mt );
}
void OSG2PRC::apply( osg::Geode& geode )
{
    std::cout << "Geode" << std::endl;

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
    std::cout << "StateSet" << std::endl;
}
void OSG2PRC::apply( const osg::Geometry* geom )
{
    std::cout << "Geometry" << std::endl;
}
