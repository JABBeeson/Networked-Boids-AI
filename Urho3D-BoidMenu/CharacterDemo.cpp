#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include "Player.h"

#include <Urho3D/DebugNew.h>
#include <Urho3D/Physics/PhysicsEvents.h>

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)
static const StringHash E_CLIENTCUSTOMEVENT("ClientCustomEvent");
int intVal;
// Custom remote event we use to tell the client which object they control
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
// Identifier for the node ID parameter in the event data
static const StringHash PLAYER_ID("IDENTITY");
// Custom event on server, client has pressed button that it wants to start game
static const StringHash E_CLIENTISREADY("ClientReadyToStart");


CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false)
{
	//TUTORIAL: TODO
}
CharacterDemo::~CharacterDemo()
{
}
void CharacterDemo::Start()
{
    // Execute base class startup
    Sample::Start();
    if (touchEnabled_)
        touch_ = new Touch(context_, TOUCH_SENSITIVITY);
	OpenConsoleWindow();
	// Subscribe to necessary events
	SubscribeToEvents();
	// Set the mouse mode to use in the sample
	Sample::InitMouseMode(MM_RELATIVE);
	//CreateScene(true);
	CreateMainMenu();
	
}

void CharacterDemo::CreateScene(bool isServer)
{
	Graphics* graphics = GetSubsystem<Graphics>();
	//so we can access resources
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	scene_ = new Scene(context_);

	// Create scene subsystem components
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);

	//Create camera node and component
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>(LOCAL);
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	camera->SetFarClip(1000.0f);
	RigidBody* cameraBody = cameraNode_->CreateComponent<RigidBody>();
	cameraBody->SetCollisionLayer(2);
	CollisionShape* cameraShape = cameraNode_->CreateComponent<CollisionShape>();
	cameraShape->SetBox(Vector3::ONE);
	

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	// Create static scene content. First create a zone for ambient	lighting and fog control
	Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	if (isServer)
	{
		zone->SetFogColor(Color(1.5f, 0.5f, 0.7f));
	}
	else zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
	zone->SetFogStart(100.0f);
	zone->SetFogEnd(300.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	// Create a directional light with cascaded shadow mapping
	Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);

	//// Create the floor object
	//Node* floorNode = scene_->CreateChild("Floor", LOCAL);
	//floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
	//floorNode->SetScale(Vector3(200.0f, 1.0f, 200.0f));
	//StaticModel* object = floorNode->CreateComponent<StaticModel>();
	//object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	//object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
	//RigidBody* body = floorNode->CreateComponent<RigidBody>();
	//// Use collision layer bit 2 to mark world scenery. This is what we	will raycast against to prevent camera from going inside geometry
	//body->SetCollisionLayer(2);
	//CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
	//shape->SetBox(Vector3::ONE);

	// Create skybox. The Skybox component is used like StaticModel, but it will be always located at the camera, giving the
	// illusion of the box planes being far away. Use just the ordinary Box model and a suitable material, whose shader will
	// generate the necessary 3D texture coordinates for cube mapping
	Node* skyNode = scene_->CreateChild("Sky");
	skyNode->SetScale(500.0f); // The scale actually does not matter
	Skybox* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	// Create heightmap terrain
	Node* terrainNode = scene_->CreateChild("Terrain");
	terrainNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	Terrain* terrain = terrainNode->CreateComponent<Terrain>();
	terrain->SetPatchSize(64);
	terrain->SetSpacing(Vector3(2.0f, 0.5f, 2.0f)); // Spacing between vertices and vertical resolution of the height map
	terrain->SetSmoothing(true);
	terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
	terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	// The terrain consists of large triangles, which fits well for occlusion rendering, as a hill can occlude all
	// terrain patches and other objects behind it
	terrain->SetOccluder(true);

	RigidBody* TerrainBody = terrainNode->CreateComponent<RigidBody>();
	TerrainBody->SetCollisionLayer(2);
	CollisionShape* TerrainShape = terrainNode->CreateComponent<CollisionShape>();
	TerrainShape->SetTerrain();

	//WaterPlane
	waterNode_ = scene_->CreateChild("Water");
	waterNode_->SetScale(Vector3(2048.0f, 1.0f, 2048.0f));
	waterNode_->SetPosition(Vector3(0.0f, 60.0f, 0.0f));
	waterNode_->SetRotation(Quaternion(180.0f, 0.0f, 0.0f));
	StaticModel* water = waterNode_->CreateComponent<StaticModel>();
	water->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
	water->SetMaterial(cache->GetResource<Material>("Materials/Water.xml"));
	
	RigidBody* waterBody = waterNode_->CreateComponent<RigidBody>();
	waterBody->SetCollisionLayer(2);
	CollisionShape* waterShape = waterNode_->CreateComponent<CollisionShape>();
	waterShape->SetBox(Vector3::ONE);
	
	// Set a different viewmask on the water plane to be able to hide it from the reflection camera
	water->SetViewMask(0x80000000);

	// Create a mathematical plane to represent the water in calculations
	waterPlane_ = Plane(waterNode_->GetWorldRotation() * Vector3(0.0f, 1.0f, 0.0f), waterNode_->GetWorldPosition());

	//Create a downward biased plane for reflection view clipping. Biasing is necessary to avoid too aggressive //clipping
	waterPlane_ = Plane(waterNode_->GetWorldRotation() * Vector3(0.0f, 1.0f, 0.0f), waterNode_->GetWorldPosition() - Vector3(0.0f, 0.01f, 0.0f));

	// Create camera for water reflection
	// It will have the same farclip and position as the main viewport camera, but uses a reflection plane to modify
	// its position when rendering
	reflectionCameraNode_ = cameraNode_->CreateChild();
	Camera* reflectionCamera = reflectionCameraNode_->CreateComponent<Camera>();
	reflectionCamera->SetFarClip(0.0);
	reflectionCamera->SetViewMask(0x7fffffff); // Hide objects with only bit 31 in the viewmask (the water plane)
	reflectionCamera->SetAutoAspectRatio(true);
	reflectionCamera->SetUseReflection(true);
	reflectionCamera->SetReflectionPlane(waterPlane_);
	reflectionCamera->SetUseClipping(true); // Enable clipping of geometry behind water plane
	reflectionCamera->SetClipPlane(waterClipPlane_);
	// The water reflection texture is rectangular. Set reflection camera aspect ratio to match
	reflectionCamera->SetAspectRatio((float)graphics->GetWidth() / (float)graphics->GetHeight());
	// View override flags could be used to optimize reflection rendering. For example disable shadows
	//reflectionCamera->SetViewOverrideFlags(VO_DISABLE_SHADOWS);

	// Create a texture and setup viewport for water reflection. Assign the reflection texture to the diffuse
	// texture unit of the water material
	int texSize = 1024;
	SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
	renderTexture->SetSize(texSize, texSize, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
	renderTexture->SetFilterMode(FILTER_BILINEAR);
	RenderSurface* surface = renderTexture->GetRenderSurface();
	SharedPtr<Viewport> rttViewport(new Viewport(context_, scene_, reflectionCamera));
	surface->SetViewport(0, rttViewport);
	Material* waterMat = cache->GetResource<Material>("Materials/Water.xml");
	waterMat->SetTexture(TU_DIFFUSE, renderTexture);


	if (isServer) {
		/*for (int i = 0; i < numOfBoidSets; i++) {
			BoidSet newBoidSet;
			newBoidSet.Initialise(cache, scene_, i);
			boidSetsA.push_back(newBoidSet);
		}*/
		for (int i = 0; i < numOfBoidSets; i++) {
			BoidSet newBoidSet;
			newBoidSet.Initialise(cache, scene_, i, "F", i);
			boidSetsB.push_back(newBoidSet);
		}

		for (int i = 0; i < numOfBoidSets; i++) {
			BoidSet newBoidSet;
			newBoidSet.Initialise(cache, scene_, i, "F", i);
			boidSetsA.push_back(newBoidSet);
		}

		const unsigned NUM_BOXES = 2;
		for (unsigned i = 0; i < NUM_BOXES; ++i)
		{
			float scale = Random(2.0f) + 0.5f;
			Node* objectNode = scene_->CreateChild("Box");
			objectNode->SetPosition(boidSetsB.at(0).goalList[i]);
			objectNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
			objectNode->SetScale(scale);
			StaticModel* object = objectNode->CreateComponent<StaticModel>();
			object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
			object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
			object->SetCastShadows(true);
			RigidBody* body = objectNode->CreateComponent<RigidBody>();
			body->SetCollisionLayer(2);
			// Bigger boxes will be heavier and harder to move
			body->SetMass(0.0f);
			CollisionShape* shape = objectNode->CreateComponent<CollisionShape>();
			shape->SetBox(Vector3::ONE);
		}
	}


}
void CharacterDemo::CreateCharacter()
{
	//TUTORIAL: TODO
}
void CharacterDemo::CreateInstructions()
{
	//TUTORIAL: TODO
}
void CharacterDemo::CreateMainMenu() 
{
	InitMouseMode(MM_RELATIVE);
	
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(uiStyle);

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);

	window_ = new Window(context_);
	root->AddChild(window_);

	window_->SetMinWidth(384);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("Window");
	window_->SetStyleAuto();

	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
	
	LineEdit* lineEditIPAddress = CreateLineEdit("IP Address", 24, window_);
	Button* buttonConnect = CreateButton(font, "Connect", 24, window_);
	Button* buttonDisconnect = CreateButton(font, "Disconnect", 24, window_);
	Button* buttonStartServer = CreateButton(font, "Start Server", 24, window_);
	Button* buttonStartGame = CreateButton(font, "Client: Start Game", 24, window_);
	Button* buttonQuit = CreateButton(font, "QuitButton", 24, window_);

	SubscribeToEvent(buttonConnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleConnect));
	SubscribeToEvent(buttonDisconnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleDisconnect));
	SubscribeToEvent(buttonStartServer, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleStartServer));
	SubscribeToEvent(buttonStartGame, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleClientStartGame));
	SubscribeToEvent(buttonQuit, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleQuit));
}
Node* CharacterDemo::CreateControllableObject() 
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	//create the scene node & visual representaion this will be a replicated object.
	Node* ballNode = scene_->CreateChild("AClientBall");
	ballNode->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	ballNode->SetScale(2.5f);
	StaticModel* ballObject = ballNode->CreateComponent<StaticModel>();
	ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	ballObject->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));
	// Create the physics components
	RigidBody* body = ballNode->CreateComponent<RigidBody>();
	body->SetMass(1.0f);
	body->SetFriction(1.0f);
	body->SetUseGravity(false);
	// motion damping so that the ball can not accelerate limitlessly
	body->SetLinearDamping(0.25f);
	body->SetAngularDamping(0.25f);
	CollisionShape* shape = ballNode->CreateComponent<CollisionShape>();
	shape->SetSphere(1.0f);
	return ballNode;

}

void CharacterDemo::HandleConnect(StringHash eventType, VariantMap& eventData)
{
	CreateScene(false);
	Network* network = GetSubsystem<Network>();
	//String address = lineEditIPAddress->GetText().Trimmed();
	String address = "localhost";
	if (address.Empty())
		address = "localhost";
	network->Connect(address, SERVER_PORT, scene_);

}
void CharacterDemo::HandleDisconnect(StringHash eventType, VariantMap& eventData) 
{
	Log::WriteRaw("(HandleDisconnect)\n");
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();
	//running as client
	if(serverConnection)
	{
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		//clientObjectID = 0;	 TODO
	}
	else if (network->IsServerRunning())
	{
		network->StopServer();
		scene_->Clear(true, false);
	}
}
void CharacterDemo::HandleStartServer(StringHash eventType, VariantMap& eventData)
{
	
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);
	CreateScene(true);	//isServer
	// code to make your main menu disappear. Boolean value
	menuVisible_ = !menuVisible_;
	Log::WriteRaw("(HandleStartServer called) Server is started!");
	//isServer = true;
}
void CharacterDemo::HandleQuit(StringHash eventType, VariantMap& eventData)
{
	engine_->Exit();
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;
	
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	float frameTime = eventData[P_TIMESTEP].GetFloat();
	
	if (GetSubsystem<UI>()->GetFocusElement()) return;	// Do not move if the UI has a focused element (the console)
	Input* input = GetSubsystem<Input>();
	
	IntVector2 mouseMove = input->GetMouseMove();	// Use this frame's mouse motion to adjust camera node yaw and pitch.
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);	// Clamp the pitch between -90 and 90 degrees

	// Construct new orientation for the camera scene node from
	// yaw and pitch. Roll is fixed to zero
	if (scene_ != NULL)
	{	
		if(clientObjectID_ == 0)
		{
			cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
			if (input->GetKeyDown(KEY_W))
				cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * frameTime);
			if (input->GetKeyDown(KEY_S))
				cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * frameTime);
			if (input->GetKeyDown(KEY_A))
				cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * frameTime);
			if (input->GetKeyDown(KEY_D))
				cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * frameTime);
		}
		else
		{
			MoveCamera();
			cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
		}

		if (serverConnection) {
			
		}

		if (network->IsServerRunning()) {
			for(int i = 0; i < boidSetsB.size(); i++)
			{
				timer += frameTime;
				timeCounter++;

				//Log::WriteRaw(String(timer) + " \n");
				if (timer > 10.0f)
				{
					timer = timer / timeCounter;
					Log::WriteRaw(String(timer) + " ");
					timer = 0.0f;
					timeCounter = 0;
				}
				if(shouldUpdateBoidGroupA)
				boidSetsA.at(i).Update(frameTime);
				else 
				boidSetsB.at(i).Update(frameTime);

				//Drawable::IsInView();
	
			}
			shouldUpdateBoidGroupA ? shouldUpdateBoidGroupA = false : shouldUpdateBoidGroupA = true;
		}
	}
	if (input->GetKeyPress(KEY_M))
	{
		menuVisible_ = !menuVisible_;
		UI* ui = GetSubsystem<UI>();
		Input* input = GetSubsystem<Input>();
		ui->GetCursor()->SetVisible(menuVisible_);
		window_->SetVisible(menuVisible_);
	}

}
void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	// menu visible & invisible
	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(menuVisible_);
	window_->SetVisible(menuVisible_);

	if (!ui->GetCursor()->IsVisible())
	{
		// ignore any keyboard input for the movement
		//[…]
	}

}
void CharacterDemo::HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData) {
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	if (serverConnection)
	{
		serverConnection->SetPosition(cameraNode_->GetPosition());		//send Camera positon 
		serverConnection->SetControls(FromClientToServerControls());	//send controls to server

		//VariantMap remoteEventData;
		//remoteEventData["aValueRemoteValue"] = intVal;	
		//serverConnection->SendRemoteEvent(E_CLIENTCUSTOMEVENT, true, remoteEventData);
	}

	//Server: Read Contorls, Apply them if needed
	else if (network->IsServerRunning())
	{
		ProcessClientControls();	// take data from clients, process it
	}
}

void CharacterDemo::HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData)
{
	printf("Event sent by the Client and running on Server: Client is ready to start the game \n");
	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	// Create a controllable object for that client
	Node* newObject = CreateControllableObject();
	SubscribeToEvent(newObject, E_NODECOLLISION, URHO3D_HANDLER(CharacterDemo, HandleNodeCollision));
	serverObjects_[newConnection] = newObject;
	// Finally send the object's node ID using a remote event
	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = newObject->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}
void CharacterDemo::HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData)
{
	clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	printf("Client ID : %i \n", clientObjectID_);
}
void CharacterDemo::HandleClientStartGame(StringHash eventType, VariantMap& eventData)
{
	printf("Client has pressed START GAME \n");
	printf("Client ID: " + clientObjectID_);
	if (clientObjectID_ == 0) // Client is still observer
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection();
		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
		}
	}
}
void CharacterDemo::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
	//To do, rework entire network modules from 3weeks ago.
	Log::WriteRaw("(HandleClientConnected) A client has connected!");
	using namespace ClientConnected;

	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);

	////when client connects, assign to a scene
	//Network* network = GetSubsystem<Network>();
	////String address = lineEditIPAddress->GetText().Trimmed();
	//String address = "localhost";
	//if (address.Empty())
	//	address = "localhost";
	//network->Connect(address, SERVER_PORT, scene_);
	//
	
	//isServer = false;
	
	//send an event to the client that has just connected
	/*VariantMap remoteEventData;
	remoteEventData["aValueRemoteValue"] = 0;
	newConnection->SendRemoteEvent(E_CLIENTCUSTOMEVENT, true, remoteEventData);*/
	//or send to all clients:
	//network->BroadcastRemoteEvent(E_CLIENTCUSTOMEVENT, true, remoteEventData);

}
void CharacterDemo::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
	using namespace ClientConnected;
	Log::WriteRaw("(Disconnected) Player has disconnected");
}
void CharacterDemo::ProcessClientControls() {
	/*
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
	//go through every client connected
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		Connection* connection = connections[i];
		const Controls& controls = connection->GetControls();
		if (controls.buttons_ & CTRL_FORWARD)
			printf("Received from Client: Controls buttons FORWARD \n");
		if (controls.buttons_ & CTRL_BACK)
			printf("Received from Client: Controls buttons BACK \n");
		if (controls.buttons_ & CTRL_LEFT)
			printf("Received from Client: Controls buttons LEFT \n");
		if (controls.buttons_ & CTRL_RIGHT)
			printf("Received from Client: Controls buttons RIGHT \n");
		if (controls.buttons_ & 1024)
			printf("Received from client: E pressed \n");
	}
	*/

	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
	//Server: go through every client connected
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
	Connection* connection = connections[i];
	// Get the object this connection is controlling
	Node* ballNode = serverObjects_[connection];
	// Client has no item connected
	if (!ballNode) continue;
	RigidBody* body = ballNode->GetComponent<RigidBody>();
	// Get the last controls sent by the client
	const Controls& controls = connection->GetControls();
	// Torque is relative to the forward vector
	Quaternion rotation(0.0f, controls.yaw_, controls.pitch_);
	const float MOVE_TORQUE = 5.0f;
	if (controls.buttons_ & CTRL_FORWARD) {
		Vector3 force = rotation * Vector3::FORWARD * MOVE_TORQUE;
		//Log::WriteRaw(String(force));
		body->ApplyForce(rotation * Vector3::FORWARD * MOVE_TORQUE);
	}
	if (controls.buttons_ & CTRL_BACK)
	body->ApplyForce(rotation * Vector3::BACK * MOVE_TORQUE);
	if (controls.buttons_ & CTRL_LEFT)
	body->ApplyForce(rotation * Vector3::LEFT * MOVE_TORQUE);
	if (controls.buttons_ & CTRL_RIGHT)
	body->ApplyForce(rotation * Vector3::RIGHT * MOVE_TORQUE);


	}
}

Controls CharacterDemo::FromClientToServerControls() {
	Input* input = GetSubsystem<Input>();
	Controls controls;
	//Check which button has been pressed, keep track
	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(1024, input->GetKeyDown(KEY_E));
	// mouse yaw to server
	controls.yaw_ = yaw_;
	return controls;
}
void CharacterDemo::MoveCamera()
{
	if (clientObjectID_)
	{
		Node* ballNode = this->scene_->GetNode(clientObjectID_);
		if (ballNode)
		{
			const float CAMERA_DISTANCE = 10.0f;
			cameraNode_->SetPosition(ballNode->GetPosition() + cameraNode_->GetRotation() * Vector3::BACK * CAMERA_DISTANCE);
		}
	}
}

void CharacterDemo::SubscribeToEvents()
{
	// Subscribe to Update event for setting the character controls
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));
	
	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientConnected));

	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientDisconnected));

	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(CharacterDemo, HandlePhysicsPreStep));
	//Server: This is called when a client has connected to a Server
	SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(CharacterDemo, HandleClientFinishedLoading));

	SubscribeToEvent(E_CLIENTCUSTOMEVENT, URHO3D_HANDLER(CharacterDemo, HandleCustomEvent));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTCUSTOMEVENT);

	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(CharacterDemo, HandleClientToServerReadyToStart));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);

	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(CharacterDemo, HandleServerToClientObjectID));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);

}

void CharacterDemo::HandleCustomEvent(StringHash eventType, VariantMap& eventData) {
	//int exampleValue = eventData["aValueRemoteValue"].GetUInt();
	//printf("This is a custom event - passed value - %i \n", exampleValue);
	
}
void CharacterDemo::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	Node* node = static_cast<Node*>(eventData["Node"].GetPtr());
	Node* otherNode = static_cast<Node*>(eventData["OtherNode"].GetPtr());
	String name = String(otherNode->GetName());	
	if (name.At(0) == *"F")
	{
		Log::WriteRaw("collided with fish");

		int setNum = (int)name.At(1)- 48;
		int eleNum = (int)name.At(2)-48;
		
		if (name.Length() > 3) {
			int eleNum2 = (int)name.At(3) - 48;
			eleNum = (eleNum * 10) + eleNum2;
		}
		boidSetsB.at(setNum).boidList.at(eleNum).Kill();
	}
}
void CharacterDemo::HandleKill(StringHash eventType, VariantMap& eventData) {
	//int exampleValue = eventData["aValueRemoteValue"].GetUInt();
	//printf("This is a custom event - passed value - %i \n", exampleValue);

}
void CharacterDemo::HandleClientFinishedLoading(StringHash eventType, VariantMap& eventData) {
	printf("Client has finished loading up the scene from the server \n");
}

Button* CharacterDemo::CreateButton(Font* font, const String& text, int pHeight, Urho3D::Window* whichWindow)
{
	Button* button = whichWindow->CreateChild<Button>();
	button->SetMinHeight(pHeight);
	button->SetStyleAuto();

	Text* buttonText = button->CreateChild<Text>();
	buttonText->SetFont(font, 12);
	buttonText->SetAlignment(HA_CENTER, VA_CENTER);
	buttonText->SetText(text);
	whichWindow->AddChild(button);
	return button;
}
LineEdit* CharacterDemo::CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow)
{
	LineEdit* lineEdit = window_->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(24);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText("localhost");
	window_->AddChild(lineEdit);
	lineEdit->SetStyleAuto();
	return lineEdit;
}


