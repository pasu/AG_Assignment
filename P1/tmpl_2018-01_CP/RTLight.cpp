#include "RTLight.h"
#include "precomp.h"

RTLight::RTLight( int _color, float _power ) : color( _color ), power( _power )
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
	PointLight( int _color, float _power, vec3 _pos ) : RTLight( _color, _power ), pos( _pos )
	{
	}
	bool castShadow( vec3 p, Scene& scene ) override
	{
		return false;
	}
};

class ParrallelLight : public RTLight
{
  private:
	vec3 direction;

  public:
	ParrallelLight( int _color, float _power, vec3 _direction ) : RTLight( _color, _power ), direction( _direction )
	{
	}
	bool castShadow(vec3 p, Scene& scene) override
	{
		return false;
	}
};