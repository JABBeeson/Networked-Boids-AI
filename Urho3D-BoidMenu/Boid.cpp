#include "Boid.h"

float Boid::Range_FAttract = 25.0f;
float Boid::Range_FRepel = 3.0f;
float Boid::Range_FAlign = 2.0f;
float Boid::Range_FGoal = 5.0f;

float Boid::FAttract_Vmax = 4.0f;

float Boid::FAttract_Factor = 70.0f;
float Boid::FRepel_Factor = 100.0f;
float Boid::FAlign_Factor = 3.0f;
float Boid::FGoal_Factor = 5.0f;

Boid::Boid(Vector3 _position, Quaternion _rotation, int _scale) {
	position = _position;
	rotation = _rotation;
	scale = _scale;
}

Boid::~Boid() {
	// set null
}

void Boid::Kill() {
	this->pRigidBody->SetPosition(Vector3(0.0f, -30.0f, 0.0f));
		Range_FAttract = 0.0f;
}

void Boid::Initialise(ResourceCache *pRes, Scene *pScene, String name, int boidSet, int boidElement, int jumps_) {
	jumps = jumps_;
	Node* boidNode = pScene->CreateChild("Element");
	boidNode->SetPosition(position);
	boidNode->SetRotation(rotation);
	boidNode->SetScale(scale);
	boidNode->SetName(name+String(boidSet)+String(boidElement));
	StaticModel* object = boidNode->CreateComponent<StaticModel>();
	object->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	object->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
	object->SetCastShadows(true);
	pRigidBody = boidNode->CreateComponent<RigidBody>();
	pRigidBody->SetUseGravity(false);
	//RigidBody->SetPosition(position);
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetMass(10.0f);
	CollisionShape* shape = boidNode->CreateComponent<CollisionShape>();
	//shape->SetTriangleMesh(object->GetModel());
	shape->SetSphere(1);
}

void Boid::ComputeForce(std::vector<Boid>* boidList, std::vector<int>* boidGrid, int currnetBoidCounter) { 
	force = Vector3(0, 0, 0);
	Vector3 fSeperation, fCohesion, fAlignment, fGoal;
	Vector3 pMean, vMean;
	int pN = 0, vN = 0;
	for (int i = 0; i < boidList->size(); i+=jumps)
	{
		if (currnetBoidCounter != i) {
			Boid otherBoid = boidList->at(i);
			Vector3 deltaP = this->pRigidBody->GetPosition() - otherBoid.pRigidBody->GetPosition();
			float fDeltaP = deltaP.Length();

			// separation
			if (fDeltaP < Range_FRepel) {
				fSeperation += (deltaP / deltaP.Length());
				//fSeperation += deltaP / (fDeltaP * fDeltaP);
			}

			// cohesion
			if (fDeltaP < Range_FAttract) {
				pMean += otherBoid.pRigidBody->GetPosition();
				pN++;
			}

			// alignment
			if (fDeltaP < Range_FAlign) {
				vMean += otherBoid.pRigidBody->GetLinearVelocity();
				vN++;
			}
		}
	}

	fSeperation *= FRepel_Factor;
	fGoal = FGoal_Factor*((((goal - this->pRigidBody->GetPosition()) / ((goal - this->pRigidBody->GetPosition()).Length()))*FGoal_Factor) - this->pRigidBody->GetLinearVelocity());

	if (pN != 0)
	{
		pMean /= pN;
		fCohesion = FAttract_Factor*((((pMean - this->pRigidBody->GetPosition()) / ((pMean - this->pRigidBody->GetPosition()).Length()))*FAttract_Vmax) - this->pRigidBody->GetLinearVelocity());
	}
	//Ray ray = Ray(this->pRigidBody->GetPosition(), otherboidPosition);
	
	if (vN != 0) 
		vMean /= vN;
	
	fAlignment = FAlign_Factor*(vMean - this->pRigidBody->GetLinearVelocity());

	force = fCohesion + fSeperation + fAlignment + fGoal;
	//Alignment breaks schooling
	//force = fSeperation;
}

void Boid::Update(float frameTime) {
	pRigidBody->ApplyForce(force);
	velocity = pRigidBody->GetLinearVelocity();
	float d = velocity.Length();
	if (d < 10.0f)
	{
		d = 10.0f;
		pRigidBody->SetLinearVelocity(velocity.Normalized()*d);
	}
	else if (d > 50.0f)
	{
		d = 50.0f;
		pRigidBody->SetLinearVelocity(velocity.Normalized()*d);
	}
	Vector3 vn = velocity.Normalized();
	Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
	float dp = cp.DotProduct(vn);
	pRigidBody->SetRotation(Quaternion(Acos(dp), cp));
	Vector3 p = pRigidBody->GetPosition();
	if (p.y_ < 10.0f)
	{
		p.y_ = 10.0f;
		pRigidBody->SetPosition(p);
	}
	else if (p.y_ > 50.0f)
	{
		p.y_ = 50.0f;
		pRigidBody->SetPosition(p);
	}

}

bool Boid::AtGoal() {
	float count = pRigidBody->GetPosition().Length() - goal.Length();
	if (count < Range_FGoal && count > -Range_FGoal)
		return true;
	else
		return false;
}