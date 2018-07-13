#pragma once
#include "Boid.h"

using namespace Urho3D;

class BoidSet
{
	String type;
public:
	int numOfBoidEle =50, jumps;
	Vector3 position, centerOfMass;
	std::vector<int> boidGrid;
	Quaternion rotation;
	int scale = 1;
	int goalNumber;
	std::vector<Boid> boidList;
	Vector3 goalList[6] = { Vector3(-30.0f, 20.0f, 0.0f), Vector3(50.0f, 20.0f, 30.0f), Vector3(30.0f, 35.0f, 40.0f), Vector3(-20.0f, 40.0f, 20.0f), Vector3(20.0f, 30.0f, 0.0f), Vector3(0.0f, 20.0f, 20.0f) };
	BoidSet();
	BoidSet(int _numOfBoidEle);
	~BoidSet();
	void Initialise(ResourceCache* pRes, Scene* pScene, int i, String name, int boidset); 
	void Update(float frameTime);
};
