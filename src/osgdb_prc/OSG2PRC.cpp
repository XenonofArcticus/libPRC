
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


	PRC3DTess *tess = createTess( geom );
	if( tess == NULL )
	{
		std::cerr << "Failed to create 3D Tess object." << std::endl;
		return;
	}

	// NOTE: this is used on the face object for starting triangle index
	//		 int works in triangles which each actually have 3 indexs for each vert, but we only worry about triangle count
	uint32_t curTriCount = 0; 
	for( unsigned int idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
    {
        const osg::PrimitiveSet* ps( geom->getPrimitiveSet( idx ) );
        switch( ps->getType() ) {
        case osg::PrimitiveSet::DrawArraysPrimitiveType:
        {
            processDrawArrays( static_cast< const osg::DrawArrays* >( ps ), tess, curTriCount );
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

	// add the tess mesh then use it
	const uint32_t tess_index = _prcFile->add3DTess( tess );
	_prcFile->useMesh( tess_index, getStyle() );
}

/////////////////////////////////////////////////////////////////////////////////////////////
PRC3DTess* OSG2PRC::createTess( const osg::Geometry* geom )
{
	PRC3DTess *tess = new PRC3DTess();

	const osg::Array* array( geom->getVertexArray() );
    if( array->getType() != osg::Array::Vec3ArrayType )
    {
        std::cerr << "Unsupported array type." << std::endl;
		delete tess;
        return NULL;
    }
    const osg::Vec3Array* vertices( static_cast< const osg::Vec3Array* >( array ) );
    std::cout << "Adding vertex array to PRC, size " << array->getNumElements() << std::endl;

	tess->coordinates.reserve( vertices->size()*3 );
	for( uint32_t i=0; i<vertices->size(); i++ )
	{
		osg::Vec3d v = vertices->at( i );
		tess->coordinates.push_back(v.x());
		tess->coordinates.push_back(v.y());
		tess->coordinates.push_back(v.z());
	}

	array = geom->getNormalArray();
    if( array != NULL )
    {
        if( array->getType() != osg::Array::Vec3ArrayType )
        {
            std::cerr << "Unsupported array type." << std::endl;
			delete tess;
            return NULL;
        }
        const osg::Vec3Array* normals( static_cast< const osg::Vec3Array* >( array ) );
        std::cout << "Adding normals array to PRC, size " << array->getNumElements() << std::endl;

		tess->normal_coordinate.reserve( normals->size()*3 );
		for( uint32_t i=0; i<normals->size(); i++ )
		{
			osg::Vec3d v = normals->at( i );
			tess->normal_coordinate.push_back(v.x());
			tess->normal_coordinate.push_back(v.y());
			tess->normal_coordinate.push_back(v.z());
		}
    }
	else
	{
		// not sure what this is for no normals
		tess->crease_angle = 25.8419;  // arccos(0.9), default found in Acrobat output; 
	}

	// TODO: support texture coordinates
	/*
	if( textured )
	{
		tess->texture_coordinate.reserve(2*nT);
		for(uint32_t i=0; i<nT; i++)
		{
			tess->texture_coordinate.push_back(T[i][0]);
			tess->texture_coordinate.push_back(T[i][1]);
		}
	}
	*/
	return tess;
}

void OSG2PRC::processDrawArrays( const osg::DrawArrays* da, PRC3DTess *tess, uint32_t &curTriCount )
{
    std::cerr << "DrawArrays not yet implemented." << std::endl;

	bool has_normals = false;	// TODO: deal with normals
	bool textured = false;		// TODO: deal with textured
	PRCTessFace *tessFace = new PRCTessFace();
	tessFace->number_of_texture_coordinate_indexes = textured ? 1 : 0;

	uint32_t triCount = 0;
    const unsigned int first( da->getFirst() );
    const unsigned int lastPlusOne( da->getFirst() + da->getCount() );
    switch( da->getMode() )
    {
    case GL_TRIANGLES:
    {
		tessFace->used_entities_flag = textured ? PRC_FACETESSDATA_TriangleTextured : PRC_FACETESSDATA_Triangle;

        for( unsigned int idx = first; idx+2 < lastPlusOne; )
        {
			
            // Triangle: idx, idx+1, idx+2

			// TODO:
			/*
			// triangle vert0 indexs
			if( has_normals )
				tess->triangulated_index.push_back( ni0 );
			if(textured)
				tess->triangulated_index.push_back( ti0 );
			tess->triangulated_index.push_back( pi0 );

			// triangle vert1 indexs
			if( has_normals )
				tess->triangulated_index.push_back( ni1 );
			if(textured)
				tess->triangulated_index.push_back( ti1 );
			tess->triangulated_index.push_back( pi1 );

			// triangle vert2 indexs
			if( has_normals )
				tess->triangulated_index.push_back( ni2 );
			if(textured)
				tess->triangulated_index.push_back( ti2 );
			tess->triangulated_index.push_back( pi2 );
			*/

            idx += 3;
			triCount++;
        }
        break;
    }
    case GL_TRIANGLE_FAN:
    {
		tessFace->used_entities_flag = textured ? PRC_FACETESSDATA_TriangleFanTextured : PRC_FACETESSDATA_TriangleFan;

        for( unsigned int idx = first+1; idx+1 < lastPlusOne; )
        {
            // Triangle: first, idx, idx+1
            idx += 1;
        }
        break;
    }
    case GL_TRIANGLE_STRIP:
    {
		// assuming STRIP is a STRIP
		tessFace->used_entities_flag = textured ? PRC_FACETESSDATA_TriangleStripeTextured : PRC_FACETESSDATA_TriangleStripe;
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
		// probably just easier to make all triangles, because we can't have multiple fans, so just add the extra index data, 6 indexes for a quad
		tessFace->used_entities_flag = textured ? PRC_FACETESSDATA_TriangleTextured : PRC_FACETESSDATA_Triangle;
        for( unsigned int idx = first; idx+3 < lastPlusOne; )
        {
            // Quad: idx, idx+1, idx+2, idx+3
            idx += 4;
        }
        break;
    }
    default:
        std::cerr << "Unsupported mode " << std::hex << da->getMode() << std::dec << std::endl;
		delete tessFace;
		return;
    }


	// update our face object
	tessFace->sizes_triangulated.push_back(triCount);
	tessFace->start_triangulated = curTriCount;
	tess->addTessFace(tessFace);

	curTriCount += triCount;
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