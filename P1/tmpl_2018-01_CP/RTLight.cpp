#include "RTLight.h"
#include "precomp.h"

RTLight::RTLight( vec3 _color, float _power ) : color( _color ), power( _power )
{
}

RTLight::~RTLight()
{
}



class PointLight : public RTLight
{
  private:
	vec3 pos;

  public:
	PointLight( vec3 _color, float _power, vec3 _pos ) : RTLight( _color, _power ), pos( _pos )
	{
	}
	vec3 shade( const SurfacePointData &pd, const RayTracer &rt, const RTMaterial &material )override
	{
		vec3 d = pos - pd.position;
		float l2 = d.sqrLentgh();
		float l = sqrt( l2 );
		vec3 nd = -d * ( 1 / l );

		RTRay ray = RTRay( pd.position, nd );
		const RTIntersection &intersection = rt.findNearestObjectIntersection( ray );

		if ( intersection.rayT < l )
		{
			return nd.dot( pd.normal ) / l2 * material.getAlbedoAtPoint( pd.textureCoordinates.x, pd.textureCoordinates.y );
		}
	}
};

class ParrallelLight : public RTLight
{
  private:
	vec3 direction;

  public:
	ParrallelLight( vec3 _color, float _power, vec3 _direction ) : RTLight( _color, _power ), direction( _direction )
	{
	}
	vec3 shade( const SurfacePointData &pd, const RayTracer &rt, const RTMaterial &material ) override
	{
		return vec3( 1.0, 0, 0 );
	}
};


RTLight *RTLight::createPointLight( vec3 _color, float _power, vec3 _pos )
{
	return new PointLight( _color, _power, _pos );
}
RTLight *RTLight::createParralleLight(vec3 _color, float _power, vec3 _direction)
{
	return new ParrallelLight( _color, _power, _direction );
}