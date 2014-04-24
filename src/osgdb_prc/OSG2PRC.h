
#ifndef __OSG_2_PRC_H__
#define __OSG_2_PRC_H__ 1

#include <osg\NodeVisitor>

namespace osg {
    class MatrixTransform;
    class Geode;
    class Geometry;
    class StateSet;
}


/** \class OSG2PRC OSG2PRC.h
\brief OSG NodeVisitor that interfaces with libPRC.
\details TBD
*/
class OSG2PRC : public osg::NodeVisitor
{
public:
    OSG2PRC();
    virtual ~OSG2PRC();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::MatrixTransform& mt );
    virtual void apply( osg::Geode& geode );

protected:
    void apply( const osg::StateSet* stateSet );
    void apply( const osg::Geometry* geom );
};


// __OSG_2_PRC_H__
#endif
