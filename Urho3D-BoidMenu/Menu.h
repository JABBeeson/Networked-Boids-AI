#pragma once
#include "Includes.h"
#include "Sample.h"
#include "Touch.h"

namespace Urho3D {
	//class UI;
}

using namespace Urho3D;
class Menu
{
public:

	ResourceCache* cache;
	UI* ui;
	UIElement * root;
	XMLFile* uiStyle;
	Window window_;

	//URHO3D_OBJECT()


	 Menu();
	~ Menu();
	void CreateMainMenu();

private:
	
};

 Menu:: Menu()
{
}

 Menu::~ Menu()
{
}

 void Menu::CreateMainMenu()
 {
	 //InitMouseMode(MM_RELATIVE);
	 //cache = GetSubsytem<ResourceCache>();
 }


