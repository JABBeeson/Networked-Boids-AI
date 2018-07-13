#pragma once
#include "Includes.h"

#include "Sample.h"
#include "BoidSet.h"

namespace Urho3D
{

class Node;
class Scene;

}

class Character;
class Touch;

/// Moving character example.
/// This sample demonstrates:
///     - Controlling a humanoid character through physics
///     - Driving animations using the AnimationController component
///     - Manual control of a bone scene node
///     - Implementing 1st and 3rd person cameras, using raycasts to avoid the 3rd person camera clipping into scenery
///     - Defining attributes of a custom component so that it can be saved and loaded
///     - Using touch inputs/gyroscope for iOS/Android (implemented through an external file)
class CharacterDemo : public Sample
{
    URHO3D_OBJECT(CharacterDemo, Sample);
	// Movement speed as world units per second
	const float MOVE_SPEED = 20.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.1f;

public:
	SharedPtr<Window> window_;
    /// Construct.
    CharacterDemo(Context* context);
    /// Destruct.
    ~CharacterDemo();
	//bool isServer;


    /// Setup after engine initialization and before running the main loop.
    virtual void Start();

protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    virtual String GetScreenJoystickPatchString() const { return
        "<patch>"
        "    <add sel=\"/element\">"
        "        <element type=\"Button\">"
        "            <attribute name=\"Name\" value=\"Button3\" />"
        "            <attribute name=\"Position\" value=\"-120 -120\" />"
        "            <attribute name=\"Size\" value=\"96 96\" />"
        "            <attribute name=\"Horiz Alignment\" value=\"Right\" />"
        "            <attribute name=\"Vert Alignment\" value=\"Bottom\" />"
        "            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />"
        "            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />"
        "            <attribute name=\"Hover Image Offset\" value=\"0 0\" />"
        "            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"Label\" />"
        "                <attribute name=\"Horiz Alignment\" value=\"Center\" />"
        "                <attribute name=\"Vert Alignment\" value=\"Center\" />"
        "                <attribute name=\"Color\" value=\"0 0 0 1\" />"
        "                <attribute name=\"Text\" value=\"Gyroscope\" />"
        "            </element>"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"KeyBinding\" />"
        "                <attribute name=\"Text\" value=\"G\" />"
        "            </element>"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">1st/3rd</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"F\" />"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Jump</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"SPACE\" />"
        "        </element>"
        "    </add>"
        "</patch>";
    }

private:
	float timer = 0.0f;
	int timeCounter = 0;
	// First person camera flag.
    bool firstPerson_;
	bool menuVisible_ = false;
	bool shouldUpdateBoidGroupA = true;
	int numOfBoidSets = 4;
	Node* CreateControllableObject(); // Server: Create a controllable ball
	SharedPtr<Node> waterNode_;
	SharedPtr<Node> waterNodeSurface_;
	SharedPtr<Node> reflectionCameraNode_;
	unsigned clientObjectID_ = 0; // Client: ID of own object
	HashMap<Connection*, WeakPtr<Node> > serverObjects_; // Server Client/Object HashMap
	Vector<Node*> players;
	Button* CreateButton(Font* font, const String& text, int pHeight, Urho3D::Window* whichWindow);
	LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow);
	Plane waterPlane_;
	Plane waterClipPlane_;
	std::vector<BoidSet> boidSetsA;
	std::vector<BoidSet> boidSetsB;
	Vector<int> boidSetGrid;
	// Touch utility object.
    SharedPtr<Touch> touch_;
    // The controllable character component.
    WeakPtr<Character> character_;
    

	// Which port this is running on
	static const unsigned short SERVER_PORT = 2345;
	//LineEdit* lineEditIPAddress;
    // Create static scene content.
    void CreateScene(bool isServer);
    // Create controllable character.
    void CreateCharacter();
    // Construct an instruction text to the UI.
    void CreateInstructions();
	//Create menu
	void CreateMainMenu();
	//create button
	void HandleQuit(StringHash eventType, VariantMap& eventData);
	// Subscribe to necessary events.
    void SubscribeToEvents();
	//void HandleConnect(StringHash eventType, VariantMap& eventData);
	//void HandleDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleConnect(StringHash eventType, VariantMap& eventData);
	void HandleDisconnect(StringHash eventType, VariantMap& eventData);
	void HandleStartServer(StringHash eventType, VariantMap& eventData);
	void HandleClientConnected(StringHash eventType, VariantMap& eventData);
	void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);
	
	void HandleClientStartGame(StringHash eventType, VariantMap& eventData);
	void HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData);
	void HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData);

	// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    // Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	
	Controls FromClientToServerControls();
	void MoveCamera();
	void ProcessClientControls();
	void HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData);
	void HandleClientFinishedLoading(StringHash eventType, VariantMap& eventData);
	void HandleCustomEvent(StringHash eventType, VariantMap& eventData);
	void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
	void HandleKill(StringHash eventType, VariantMap& eventData);
	
	
};
