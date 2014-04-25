
#ifndef __OSG_2_PRC_H__
#define __OSG_2_PRC_H__ 1

#include <osg\NodeVisitor>

#ifdef PRC_USE_ASYMPTOTE
#  include <oPRCFile.h>
#endif

namespace osg {
    class MatrixTransform;
    class Geode;
    class Geometry;
    class StateSet;
    class DrawArrays;
    class DrawArrayLengths;
    class DrawElementsUByte;
    class DrawElementsUShort;
    class DrawElementsUInt;
}


/** \class OSG2PRC OSG2PRC.h
\brief OSG NodeVisitor that interfaces with libPRC.
\details TBD
*/
class OSG2PRC : public osg::NodeVisitor
{
public:
    OSG2PRC();
#ifdef PRC_USE_ASYMPTOTE
	OSG2PRC( oPRCFile* prcFile );
#endif
    virtual ~OSG2PRC();

    virtual void apply( osg::Node& node );
    virtual void apply( osg::Transform& trans );
    virtual void apply( osg::Geode& geode );

protected:
    void apply( const osg::StateSet* stateSet );
    void apply( const osg::Geometry* geom );

    void processNewNode( const std::string& name );
    void processTransformNode( const std::string& name, const osg::Matrix& matrix );
	void finishNode();

    static void processDrawArrays( const osg::DrawArrays* da );
    static void processDrawArrayLengths( const osg::DrawArrayLengths* dal );
    static void processDrawElements( const osg::DrawElementsUInt* deui );
    static osg::DrawElementsUInt* convertDrawElements( const osg::DrawElementsUByte* deub );
    static osg::DrawElementsUInt* convertDrawElements( const osg::DrawElementsUShort* deus );

protected:
#ifdef PRC_USE_ASYMPTOTE

    static RGBAColour colorToPRC( const osg::Vec4& osgColor )
    {
        return( RGBAColour( osgColor[ 0 ],
            osgColor[ 1 ],osgColor[ 2 ],osgColor[ 3 ] ) );
    }

	oPRCFile* _prcFile;

#endif
};


// __OSG_2_PRC_H__
#endif
