#pragma once

class Scene;

class RTLight
{
  private:
	int color;
	float power;
  public:
	RTLight(int _color, float _power);
	~RTLight();
	virtual bool castShadow(vec3 p,Scene& scene)=0;
};
