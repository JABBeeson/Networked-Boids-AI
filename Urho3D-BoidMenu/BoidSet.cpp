#include "BoidSet.h"

BoidSet::BoidSet() {
}

BoidSet::BoidSet(int _numOfBoidEle) {
	numOfBoidEle = _numOfBoidEle;
}

BoidSet::~BoidSet() {

}

void BoidSet::Initialise(ResourceCache* pRes, Scene* pScene, int goalNumber_, String name, int boidSet) {
	goalNumber = goalNumber_;
	type = name;
	jumps = numOfBoidEle / 10;
	for (int i = 0; i < numOfBoidEle; i++) {
		position = Vector3(Random(40.0f), Random(40.0f) + 10.0f, Random(40.0f));
		//position = Vector3(0.0f, 10.0f, 0.0f);
		rotation = Quaternion(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
		Boid boid(position, rotation, 1);
		boid.Initialise(pRes, pScene, name, boidSet, i, jumps);
		if (goalNumber > 6)
			goalNumber = 0;
		boid.goal = goalList[goalNumber];
		boidList.push_back(boid);
	}
}

void BoidSet::Update(float frameTime) {
	float f= frameTime;
	for(int i = 0; i < boidList.size(); i++){
		boidList.at(i).ComputeForce(&boidList, &boidGrid, i);
		if (boidList.at(i).AtGoal())
		{
			if (goalNumber == 5)
				goalNumber = -1;
			boidList.at(i).goal = goalList[goalNumber++];
		}
		boidList.at(i).Update(frameTime);
		centerOfMass += boidList.at(i).position;	
	}
	centerOfMass /= boidList.size();
}