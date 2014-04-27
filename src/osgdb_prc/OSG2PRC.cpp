
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
    addDefaultMaterial();
}

#ifdef PRC_USE_ASYMPTOTE
OSG2PRC::OSG2PRC( oPRCFile* prcFile )
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _prcFile( prcFile )
{
    addDefaultMaterial();
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

    finishNode();
}

void OSG2PRC::pushStyle()
{
    _styleStack.resize( _styleStack.size() + 1 );
    if( _styleStack.size() > 1 )
    {
        // Copy old top of stack to current top of stack.
        _styleStack[ _styleStack.size() - 1 ] =
            _styleStack[ _styleStack.size() - 2 ];
    }
}
bool OSG2PRC::popStyle()
{
    if( _styleStack.size() > 0 )
    {
        _styleStack.resize( _styleStack.size() - 1 );
        return( true );
    }
    return( false );
}
void OSG2PRC::setStyle( const uint32_t style )
{
    if( _styleStack.size() > 0 )
        _styleStack[ _styleStack.size() - 1 ] = style;
}
uint32_t OSG2PRC::getStyle() const
{
    if( _styleStack.size() > 0 )
        return( _styleStack[ _styleStack.size() - 1 ] );
    else
        return( 0 );
}
void OSG2PRC::addDefaultMaterial()
{
    const osg::Vec4 ambient( 0.f, 0.f, 0.f, 0.f );
    const osg::Vec4 emissive( 0.f, 0.f, 0.f, 0.f );
    const osg::Vec4 diffuse( 0.7f, 0.7f, 0.7f, 0.f );
    const osg::Vec4 specular( 0.3f, 0.3f, 0.3f, 0.f );
#ifdef PRC_USE_ASYMPTOTE
    PRCmaterial m( colorToPRC( ambient ),
        colorToPRC( diffuse ),
        colorToPRC( emissive ),
        colorToPRC( specular ),
        1., 16. );

    const uint32_t style = _prcFile->addMaterial( m );
    pushStyle();
    setStyle( style );
#else
    // libPRC version
#endif
}

void OSG2PRC::apply( const osg::StateSet* stateSet )
{
    std::cout << "Found osg::StateSet" << std::endl;

    const osg::StateAttribute* sa( stateSet->getAttribute( osg::StateAttribute::MATERIAL ) );
    if( sa != NULL )
    {
        const osg::Material* mat( static_cast< const osg::Material* >( sa ) );

        MaterialStyleMap::iterator styleIt( _styles.find( mat ) );
        if( styleIt == _styles.end() )
        {
#ifdef PRC_USE_ASYMPTOTE
            const double alpha( 1. );
            const osg::Material::Face face( osg::Material::FRONT );
            PRCmaterial m( colorToPRC( mat->getAmbient( face ) ),
                colorToPRC( mat->getDiffuse( face ) ),
                colorToPRC( mat->getEmission( face ) ),
                colorToPRC( mat->getSpecular( face ) ),
                alpha, (double)( mat->getShininess( face ) ) );

            const uint32_t style = _prcFile->addMaterial( m );
            _styles[ mat ] = style;
            styleIt = _styles.find( mat );
#else
            // libPRC version
#endif
        }
        setStyle( styleIt->second );
    }
}
void OSG2PRC::apply( const osg::Geometry* geom )
{
    std::cout << "Found osg::Geometry" << std::endl;

    if( geom->getStateSet() != NULL )
        apply( geom->getStateSet() );

    std::cout << "Current style: " << getStyle() << std::endl;

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
        switch( ps->getType() ) {
        case osg::PrimitiveSet::DrawArraysPrimitiveType:
        {
            processDrawArrays( static_cast< const osg::DrawArrays* >( ps ) );
            break;
        }
        case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
        {
            processDrawArrayLengths( static_cast< const osg::DrawArrayLengths* >( ps ) );
            break;
        }
        case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
        {
            osg::ref_ptr< osg::DrawElementsUInt > deui( convertDrawElements(
                static_cast< const osg::DrawElementsUByte* >( ps ) ) );
            processDrawElements( deui.get() );
            break;
        }
        case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
        {
            osg::ref_ptr< osg::DrawElementsUInt > deui( convertDrawElements(
                static_cast< const osg::DrawElementsUShort* >( ps ) ) );
            processDrawElements( deui.get() );
            break;
        }
        case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
        {
            processDrawElements( static_cast< const osg::DrawElementsUInt* >( ps ) );
            break;
        }
        }
    }
}

void OSG2PRC::processDrawArrays( const osg::DrawArrays* da )
{
    std::cerr << "DrawArrays not yet implemented." << std::endl;

    const unsigned int first( da->getFirst() );
    const unsigned int lastPlusOne( da->getFirst() + da->getCount() );
    switch( da->getMode() )
    {
    case GL_TRIANGLES:
    {
        for( unsigned int idx = first; idx+2 < lastPlusOne; )
        {
            // Triangle: idx, idx+1, idx+2
            idx += 3;
        }
        break;
    }
    case GL_TRIANGLE_FAN:
    {
        for( unsigned int idx = first+1; idx+1 < lastPlusOne; )
        {
            // Triangle: first, idx, idx+1
            idx += 1;
        }
        break;
    }
    case GL_TRIANGLE_STRIP:
    {
        for( unsigned int idx = first; idx+2 < lastPlusOne; )
        {
            // Triangle: idx, idx+1, idx+2
            // if( idx+3 < ledtPlusOne
            //   Triangle: idx+2, idx+1, idx+3
            idx += 2;
        }
        break;
    }
    case GL_QUADS:
    {
        for( unsigned int idx = first; idx+3 < lastPlusOne; )
        {
            // Quad: idx, idx+1, idx+2, idx+3
            idx += 4;
        }
        break;
    }
    default:
        std::cerr << "Unsupported mode " << std::hex << da->getMode() << std::dec << std::endl;
        break;
    }
}
void OSG2PRC::processDrawArrayLengths( const osg::DrawArrayLengths* dal )
{
    std::cerr << "DrawArrayLengths not yet implemented." << std::endl;
}
void OSG2PRC::processDrawElements( const osg::DrawElementsUInt* deui )
{
    std::cerr << "DrawElements not yet implemented." << std::endl;
}
osg::DrawElementsUInt* OSG2PRC::convertDrawElements( const osg::DrawElementsUByte* deub )
{
    osg::ref_ptr< osg::DrawElementsUInt > deui( new osg::DrawElementsUInt() );

    deui->setMode( deub->getMode() );
    deui->setNumInstances( deub->getNumInstances() );
    for( unsigned char idx=0; idx< deub->size(); ++idx )
        deui->push_back( (unsigned int) idx );

    return( deui.release() );
}
osg::DrawElementsUInt* OSG2PRC::convertDrawElements( const osg::DrawElementsUShort* deus )
{
    osg::ref_ptr< osg::DrawElementsUInt > deui( new osg::DrawElementsUInt() );

    deui->setMode( deus->getMode() );
    deui->setNumInstances( deus->getNumInstances() );
    for( unsigned short idx=0; idx< deus->size(); ++idx )
        deui->push_back( (unsigned int) idx );

    return( deui.release() );
}


void OSG2PRC::processNewNode( const std::string& name )
{
	std::cout << "Adding new node (with name " << name << ") to PRC" << std::endl;

	_prcFile->begingroup( name.c_str() );

    pushStyle();
}
void OSG2PRC::processTransformNode( const std::string& name, const osg::Matrix& matrix )
{
    std::cout << "TBD: Add matrix to PRC" << std::endl;

	_prcFile->begingroup( name.c_str(), NULL, matrix.ptr());
}
void OSG2PRC::finishNode()
{
	_prcFile->endgroup();

    popStyle();
}