#include "precomp.h"
#include "RTObjMesh.h"


RTObjMesh::RTObjMesh( char *a_File, const RTMaterial &material ) 
	: RTPrimitive( material ), rotation( vec3( 0 ) ), scale( vec3( 1 ) )
{
	boundingBox = NULL;
	importer = new Assimp::Importer();

	importer->ReadFile( a_File, aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate |
								  aiProcess_JoinIdenticalVertices );

	if ( !importer->GetScene() )
	{
		cout << "Error loading mesh: " << a_File << ". " << importer->GetErrorString();
		return;
	}

	applyTransforms();
}

RTObjMesh::~RTObjMesh()
{
}

void RTObjMesh::applyTransforms()
{
	// mat = S * Rz * Ry * Rx * T
	modelMatrix.cell[3] = pos.x;
	modelMatrix.cell[7] = pos.y;
	modelMatrix.cell[11] = pos.z;

	mat4 rotateXYZ;
	rotateXYZ = mat4::rotatex( rotation.x );
	modelMatrix = modelMatrix * rotateXYZ;
	rotateXYZ = mat4::rotatey( rotation.y );
	modelMatrix = modelMatrix * rotateXYZ;
	rotateXYZ = mat4::rotatez( rotation.z );
	modelMatrix = modelMatrix * rotateXYZ;

	mat4 scaleXYZ;
	scaleXYZ.cell[0] = scale.x;
	scaleXYZ.cell[5] = scale.y;
	scaleXYZ.cell[10] = scale.z;
	modelMatrix = modelMatrix * scaleXYZ;

	computeNormalMatrix();

	vec3 max( Utils::MIN_FLOAT, Utils::MIN_FLOAT, Utils::MIN_FLOAT );
	vec3 min( Utils::MAX_FLOAT, Utils::MAX_FLOAT, Utils::MAX_FLOAT );

	const aiScene *scene = importer->GetScene();
	for ( unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI )
	{
		aiMesh *mesh = scene->mMeshes[meshI];
		aiVector3D *vertices = mesh->mVertices;
		aiVector3D *normals = mesh->mNormals;
		for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
		{

			//TODO: unify vector usage
			vec4 pos( vertices[i][0], vertices[i][1], vertices[i][2], 1 );
			pos = modelMatrix * pos;
			vertices[i][0] = pos.x;
			vertices[i][1] = pos.y;
			vertices[i][2] = pos.z;

			vec4 n( normals[i][0], normals[i][1], normals[i][2], 0 );
			n = normalMatrix * n;
			normals[i][0] = n.x;
			normals[i][1] = n.y;
			normals[i][2] = n.z;

			computeBounds( vertices[i], min, max );
		}
	}
	if ( boundingBox ) delete boundingBox;
	boundingBox = new RTBox( pos, min, max, material );
}

const RTIntersection RTObjMesh::intersect( const RTRay &ray ) const
{
	RTIntersection nearestIntersection;
	const aiScene *scene = importer->GetScene();

	nearestIntersection = boundingBox->intersect( ray );
	if ( nearestIntersection.isIntersecting() )
	{
		nearestIntersection.rayT = -1;

		for ( unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI )
		{
			aiMesh *mesh = scene->mMeshes[meshI];
			aiFace *faces = mesh->mFaces;
			aiVector3D *vertices = mesh->mVertices;

			for ( unsigned int faceI = 0; faceI < mesh->mNumFaces; ++faceI )
			{
				aiVector3D &a = mesh->mVertices[faces[faceI].mIndices[0]];
				aiVector3D &b = mesh->mVertices[faces[faceI].mIndices[1]];
				aiVector3D &c = mesh->mVertices[faces[faceI].mIndices[2]];
				const RTIntersection &intersection = intersectTriangle( ray, vec3( a.x, a.y, a.z ),
																	  vec3( b.x, b.y, b.z ),
																	  vec3( c.x, c.y, c.z ) );

				if ( intersection.isIntersecting() &&
					 ( !nearestIntersection.isIntersecting() || intersection.rayT < nearestIntersection.rayT ) )
				{
					nearestIntersection = intersection;
					nearestIntersection.triangleIndex = faceI;
				}
			}
		}
	}
	return nearestIntersection;
}

const SurfacePointData RTObjMesh::getSurfacePointData( const RTIntersection &intersection ) const
{
	const vec3 &point = intersection.getIntersectionPosition();

	const aiScene *scene = importer->GetScene();
	aiMesh *mesh = scene->mMeshes[0]; //TODO: mesh 0 :(
	aiFace *faces = mesh->mFaces;
	aiVector3D &na = mesh->mNormals[faces[intersection.triangleIndex].mIndices[0]];
	aiVector3D &nb = mesh->mNormals[faces[intersection.triangleIndex].mIndices[1]];
	aiVector3D &nc = mesh->mNormals[faces[intersection.triangleIndex].mIndices[2]];

	vec3 normal( na.x + intersection.u * ( nb.x - na.x ) + intersection.v * ( nc.x - na.x ),
				 na.y + intersection.u * ( nb.y - na.y ) + intersection.v * ( nc.y - na.y ),
				 na.z + intersection.u * ( nb.z - na.z ) + intersection.v * ( nc.z - na.z ) );

// 	aiVector3D &a = mesh->mVertices[faces[intersection.triangleIndex].mIndices[0]];
// 	aiVector3D &b = mesh->mVertices[faces[intersection.triangleIndex].mIndices[1]];
// 	aiVector3D &c = mesh->mVertices[faces[intersection.triangleIndex].mIndices[2]];
// 
// 	vec3 position(a.x + intersection.u * (b.x - a.x) + intersection.v * (c.x - a.x),
// 		a.y + intersection.u * (b.y - a.y) + intersection.v * (c.y - a.y),
// 		a.z + intersection.u * (b.z - a.z) + intersection.v * (c.z - a.z));

	return {normalize( normal ), {0, 0}, point};
}

const RTIntersection RTObjMesh::intersectTriangle( const RTRay &ray, const vec3 &a, const vec3 &b, const vec3 &c ) const
{
	RTIntersection intersection( &ray, this, -1 );

	vec3 AB = b - a;
	vec3 AC = c - a;

	vec3 P = cross( ray.dir, AC );
	float denominator = dot( P, AB ); //if negative triangle is backfacing (Cull here)

	if ( Utils::floatEquals( denominator, 0.0f ) ) //ray parallel to triangle
		return intersection;

	float inverseDenominator = 1.0f / denominator;
	vec3 T = ray.orig - a;
	float u = inverseDenominator * dot( P, T );
	if ( u < 0 || u > 1 )
		return intersection;

	vec3 Q = cross( T, AB );
	float v = inverseDenominator * dot( Q, ray.dir );
	if ( v < 0 || u + v > 1 )
		return intersection;

	intersection.rayT = inverseDenominator * dot( Q, AC );
	intersection.u = u;
	intersection.v = v;
	return intersection;
}

const void RTObjMesh::computeBounds( const aiVector3D &vertex, vec3 &min, vec3 &max ) const
{
	for ( int i = 0; i < 3; ++i )
	{
		if ( vertex[i] < min[i] )
			min[i] = vertex[i];
		if ( vertex[i] > max[i] )
			max[i] = vertex[i];
	}
}
