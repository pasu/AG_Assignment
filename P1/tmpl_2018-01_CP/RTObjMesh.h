#pragma once
#include "RTPrimitive.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "RTMaterial.h"
#include "RTBox.h"

class RTObjMesh : public RTPrimitive
{
  public:
	RTObjMesh( char *a_File, const RTMaterial &material );
	~RTObjMesh();

	void applyTransforms();

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;

	inline void setPosition( const float x, const float y, const float z )
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}
	inline void setRotation( const float angleX, const float angleY, const float angleZ )
	{
		rotation.x = angleX;
		rotation.y = angleY;
		rotation.z = angleZ;
	}
	inline void setScale( const float x, const float y, const float z )
	{
		scale.x = x;
		scale.y = y;
		scale.z = z;
	}

	inline void computeNormalMatrix() 
	{
		normalMatrix = modelMatrix;
		normalMatrix.invert();

		// transpose
		swap( normalMatrix.cell[1], normalMatrix.cell[4] );
		swap( normalMatrix.cell[2], normalMatrix.cell[8] );
		swap( normalMatrix.cell[3], normalMatrix.cell[12] );

		swap( normalMatrix.cell[6], normalMatrix.cell[9] );
		swap( normalMatrix.cell[7], normalMatrix.cell[13] );

		swap( normalMatrix.cell[11], normalMatrix.cell[14] );
	};

  private:
	const RTIntersection intersectTriangle( const RTRay &ray, const vec3 &a, const vec3 &b, const vec3 &c ) const;
	const void computeBounds( const aiVector3D &vertex, vec3 &min, vec3 &max ) const;

	Assimp::Importer *importer;

	const RTBox *boundingBox;

	vec3 scale;
	vec3 rotation;

	mat4 modelMatrix;  //Object to world Matrix
	mat4 normalMatrix; //Tranposed Inverse Model Matrix

};