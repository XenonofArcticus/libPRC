
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
    //std::cout << "Found osg::Node" << std::endl;

    processNewNode( node.getName() );

    if( node.getStateSet() != NULL )
        apply( node.getStateSet() );

    traverse( node );

	finishNode();
}
void OSG2PRC::apply( osg::Transform& trans )
{
    //std::cout << "Found osg::Transform" << std::endl;

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
    //std::cout << "Found osg::Geode" << std::endl;

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
    //std::cout << "Found osg::StateSet" << std::endl;

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
    //std::cout << "Found osg::Geometry" << std::endl;

    if( geom->getStateSet() != NULL )
        apply( geom->getStateSet() );

    //std::cout << "Current style: " << getStyle() << std::endl;


	PRC3DTess *tess = createTess( geom );
	if( tess == NULL )
	{
		std::cerr << "Failed to create 3D Tess object." << std::endl;
		return;
	}

	// NOTE: this is used on the face object for starting triangle index
	//		 int works in triangles which each actually have 3 indexs for each vert, but we only worry about triangle count
	uint32_t curIdxCount = 0; 
	for( unsigned int idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
    {
        const osg::PrimitiveSet* ps( geom->getPrimitiveSet( idx ) );
        switch( ps->getType() ) {
        case osg::PrimitiveSet::DrawArraysPrimitiveType:
        {
            processDrawArrays( static_cast< const osg::DrawArrays* >( ps ), tess, curIdxCount );
            break;
        }
        case osg::PrimitiveSet::DrawArrayLengthsPrimitiveType:
        {
            processDrawArrayLengths( static_cast< const osg::DrawArrayLengths* >( ps ), tess, curIdxCount );
            break;
        }
        case osg::PrimitiveSet::DrawElementsUBytePrimitiveType:
        case osg::PrimitiveSet::DrawElementsUShortPrimitiveType:
        case osg::PrimitiveSet::DrawElementsUIntPrimitiveType:
        {
            processDrawElements( static_cast< const osg::DrawElements* >( ps ), tess, curIdxCount );
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
		const osg::Vec3& v( vertices->at( i ) );
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

        tess->normal_coordinate.reserve( vertices->size()*3 );
        if( normals->size() == vertices->size() )
        {
		    for( uint32_t i=0; i<normals->size(); i++ )
		    {
			    const osg::Vec3& v( normals->at( i ) );
			    tess->normal_coordinate.push_back(v.x());
			    tess->normal_coordinate.push_back(v.y());
			    tess->normal_coordinate.push_back(v.z());
		    }
        }
        else
        {
            // Normals array is a different size, BIND_OVERALL is likely cause.
            // Let's just fake it by repeating the first normal.
            const osg::Vec3& v( normals->at( 0 ) );
		    for( uint32_t i=0; i<vertices->size(); i++ )
		    {
			    tess->normal_coordinate.push_back(v.x());
			    tess->normal_coordinate.push_back(v.y());
			    tess->normal_coordinate.push_back(v.z());
		    }
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

void OSG2PRC::processDrawArrays( const osg::DrawArrays* da, PRC3DTess* tess, uint32_t& curIdxCount )
{
	const bool hasnormals( tess->normal_coordinate.size() > 0 );
    const bool hasTexCoords( tess->texture_coordinate.size() > 0 );

	PRCTessFace *tessFace = new PRCTessFace();
	tessFace->number_of_texture_coordinate_indexes = hasTexCoords ? 1 : 0;

    // Tell the PRC tess face object what our entity type is.
    switch( da->getMode() )
    {
    case GL_QUADS:
        // Store quads as triangles.
    case GL_TRIANGLES:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleTextured : PRC_FACETESSDATA_Triangle;
        break;
    case GL_TRIANGLE_FAN:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleFanTextured : PRC_FACETESSDATA_TriangleFan;
        break;
    case GL_TRIANGLE_STRIP:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleStripeTextured : PRC_FACETESSDATA_TriangleStripe;
        break;
    default:
        std::cerr << "Unsupported mode " << std::hex << da->getMode() << std::dec << std::endl;
		delete tessFace; // we don't need this face object.. destroy it
		return;
        break;
    }

    // Add indices to the tess.
	uint32_t triCount = 0;
	uint32_t idxCount = 0;
    const unsigned int first( da->getFirst() );
    const unsigned int lastPlusOne( da->getFirst() + da->getCount() );
    switch( da->getMode() )
    {
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    {
        for( unsigned int idx = first; idx < lastPlusOne; ++idx )
        {
			if( hasnormals )
				tess->triangulated_index.push_back( 3*idx );
			if( hasTexCoords )
				tess->triangulated_index.push_back( 2*idx );
			tess->triangulated_index.push_back( 3*idx );

			idxCount++;
        }

        if( da->getMode() == GL_TRIANGLES )
			triCount = da->getCount() / 3;
        else // strip or fan
			triCount = ( da->getCount() >= 3 ) ? ( da->getCount() - 2 ) : 0;
		
        break;
    }
    case GL_QUADS:
    {
        for( unsigned int idx = first; idx+3 < lastPlusOne; idx += 4 )
        {
            const unsigned int curIdx0( idx );
            const unsigned int curIdx1( idx+1 );
            const unsigned int curIdx2( idx+2 );
            const unsigned int curIdx3( idx+3 );

            // Two triangles, A and B
            //   Triangle A, verts 0, 1, 2
			if( hasnormals )
				tess->triangulated_index.push_back( curIdx0 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx0 * 2 );
			tess->triangulated_index.push_back( curIdx0 * 3 );
			idxCount++;

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx1 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx1 * 2 );
			tess->triangulated_index.push_back( curIdx1 * 3 );
			idxCount++;

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx2 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx2 * 2 );
			tess->triangulated_index.push_back( curIdx2 * 3 );
			idxCount++;

            //   Triangle B, verts 0, 2, 3
			if( hasnormals )
				tess->triangulated_index.push_back( curIdx0 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx0 * 2 );
			tess->triangulated_index.push_back( curIdx0 * 3 );
			idxCount++;

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx2 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx2 * 2 );
			tess->triangulated_index.push_back( curIdx2 * 3 );
			idxCount++;

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx3 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx3 * 2 );
			tess->triangulated_index.push_back( curIdx3 * 3 );
			idxCount++;

			triCount += 2;
        }
        break;
    }
    default:
        break;
    }

	// update our face object
	
	switch( da->getMode() )
    {
    case GL_TRIANGLES:
	case GL_QUADS:
		tessFace->sizes_triangulated.push_back( triCount );
		break;
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
		tessFace->sizes_triangulated.push_back( 1 );
		tessFace->sizes_triangulated.push_back( idxCount );
		break;
	}
	
	
	tessFace->start_triangulated = curIdxCount;
	tess->addTessFace( tessFace );
    tess->has_faces = true;

	curIdxCount += idxCount; // update the triangle count
}
void OSG2PRC::processDrawArrayLengths( const osg::DrawArrayLengths* dal, PRC3DTess* tess, uint32_t& curIdxCount )
{
    //std::cout << "Processing DrawArrayLengths." << std::endl;

    osg::ref_ptr< osg::DrawArrays > da( new osg::DrawArrays( dal->getMode() ) );
    da->setFirst( dal->getFirst() );
    da->setNumInstances( dal->getNumInstances() );
    for( unsigned int idx=0; idx < dal->size(); ++idx )
    {
        da->setCount( (*dal)[ idx ] );
        processDrawArrays( da.get(), tess, curIdxCount );
        da->setFirst( da->getFirst() + (*dal)[ idx ] );
    }
}
void OSG2PRC::processDrawElements( const osg::DrawElements* de, PRC3DTess* tess, uint32_t& curIdxCount )
{
    //std::cout << "Processing DrawElements." << std::endl;

	const bool hasnormals( tess->normal_coordinate.size() > 0 );
    const bool hasTexCoords( tess->texture_coordinate.size() > 0 );

	PRCTessFace *tessFace = new PRCTessFace();
	tessFace->number_of_texture_coordinate_indexes = hasTexCoords ? 1 : 0;

    // Tell the PRC tess face object what our entity type is.
    switch( de->getMode() )
    {
    case GL_QUADS:
        // Store quads as triangles.
    case GL_TRIANGLES:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleTextured : PRC_FACETESSDATA_Triangle;
        break;
    case GL_TRIANGLE_FAN:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleFanTextured : PRC_FACETESSDATA_TriangleFan;
        break;
    case GL_TRIANGLE_STRIP:
		tessFace->used_entities_flag = hasTexCoords ? PRC_FACETESSDATA_TriangleStripeTextured : PRC_FACETESSDATA_TriangleStripe;
        break;
    default:
        std::cerr << "Unsupported mode " << std::hex << de->getMode() << std::dec << std::endl;
		delete tessFace; // we don't need this face object.. destroy it
		return;
        break;
    }

    // Add indices to the tess.
	uint32_t triCount = 0;
	uint32_t idxCount = 0; // TODO:
    switch( de->getMode() )
    {
    case GL_TRIANGLES:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLE_STRIP:
    {
        for( unsigned int idx = 0; idx < de->getNumIndices(); ++idx )
        {
            const unsigned int curIdx( de->index( idx ) );
			if( hasnormals )
				tess->triangulated_index.push_back( curIdx * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx * 2 );
			tess->triangulated_index.push_back( curIdx * 3 );

        }
        if( de->getMode() == GL_TRIANGLES )
            triCount = de->getNumIndices() / 3;
        else // strip or fan
			triCount = ( de->getNumIndices() >= 3 ) ? ( de->getNumIndices() - 2 ) : 0;
        break;
    }
    case GL_QUADS:
    {
        for( unsigned int idx = 0; idx+3 < de->getNumIndices(); idx += 4 )
        {
            const unsigned int curIdx0( de->index( idx ) );
            const unsigned int curIdx1( de->index( idx+1 ) );
            const unsigned int curIdx2( de->index( idx+2 ) );
            const unsigned int curIdx3( de->index( idx+3 ) );

            // Two triangles, A and B
            //   Triangle A, verts 0, 1, 3
			if( hasnormals )
				tess->triangulated_index.push_back( curIdx0 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx0 * 2 );
			tess->triangulated_index.push_back( curIdx0 * 3 );

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx1 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx1 * 2 );
			tess->triangulated_index.push_back( curIdx1 * 3 );

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx3 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx3 * 2 );
			tess->triangulated_index.push_back( curIdx3 * 3 );

            //   Triangle B, verts 3, 1, 2
			if( hasnormals )
				tess->triangulated_index.push_back( curIdx3 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx3 * 2 );
			tess->triangulated_index.push_back( curIdx3 * 3 );

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx1 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx1 * 2 );
			tess->triangulated_index.push_back( curIdx1 * 3 );

			if( hasnormals )
				tess->triangulated_index.push_back( curIdx2 * 3 );
			if( hasTexCoords )
				tess->triangulated_index.push_back( curIdx2 * 2 );
			tess->triangulated_index.push_back( curIdx2 * 3 );

			triCount += 2;
        }
        break;
    }
    default:
        break;
    }

	// TODO:
	/*
	// update our face object
	tessFace->sizes_triangulated.push_back( triCount );
	tessFace->start_triangulated = curTriCount;
	tess->addTessFace( tessFace );
    tess->has_faces = true;

	curTriCount += triCount; // update the triangle count
	*/
}


void OSG2PRC::processNewNode( const std::string& name )
{
	//std::cout << "Adding new node (with name " << name << ") to PRC" << std::endl;

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
