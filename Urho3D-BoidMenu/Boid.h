#pragma once
#include "Includes.h"
#include <vector>
#include <Urho3D\Math\Ray.h>
//namespace Urho3D
//{
//	class Node;
//	class Scene;
//	class RigidBody;
//	class CollisionShape;
//	class ResourceCache;
//}
// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;
class Boid
{
public:
	int scale, vectorIndex, jumps;
	std::vector<int> neighbourList;
	int searchList [8] = {-12, -11, -10, -1, 1, 10, 11, 12};
	Vector3 position, velocity, goal;
	Quaternion rotation;
	Vector3 force;
	Node* pNode;
	RigidBody* pRigidBody;
	CollisionShape* pCollisionShape;
	StaticModel* pStaticModel;

	Boid(Vector3 _position, Quaternion _rotation, int _scale);
	~Boid();
	bool AtGoal();
	void Kill();
	void Initialise(ResourceCache *pRes, Scene *pScene, String name, int boidSet, int boidElement, int jumps);
	void ComputeForce(std::vector<Boid> *boidList, std::vector<int> *boidGrid, int i);
	void Update(float frameTime);

private:
	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;
	static float Range_FGoal;
	static float FAttract_Vmax; // vmax
	static float FAttract_Factor; // cc
	static float FRepel_Factor; // cs
	static float FAlign_Factor;	// ca
	static float FGoal_Factor;
};

