/*
	This class is your specific game
*/

#ifndef CGAME_H
#define CGAME_H

// Header Files
//=============

#include "../../Engine/Application/cbApplication.h"

#if defined( WINDOWS_API )
	#include "Resource Files/Resource.h"
#endif

// Class Declaration
//==================

namespace Engine
{
	class cGame : public Application::cbApplication
	{
		// Interface
		//==========

	public:

		// Initialization / Clean Up
		//--------------------------

		virtual ~cGame();

		// Inherited Implementation
		//=========================

	private:

		// Configuration
		//--------------

		// You can configure your game by overriding the following functions
		
		virtual const char* GetPathToLogTo() const { return "Engine.log"; }
#if defined( WINDOWS_API )
		// The main window's name will be displayed as its caption (the text that is displayed in the title bar).
		// You can make it anything that you want, but please keep the platform name and debug configuration in it somewhere
		// so that it's easy to tell at a glance what kind of build is running.
		virtual const char* GetMainWindowName() const
		{
			return "Game"
				" -- "
#if defined( D3D_API )
				"D3D"
#elif defined( GL_API )
				"GL"
#endif
#ifdef _DEBUG
				" -- Debug"
#endif
				;
		}
		// Window classes are almost always identified by name;
		// there is a unique "ATOM" associated with them,
		// but in practice Windows expects to use the class name as an identifier.
		// If you don't change the name below
		// your program could conceivably have problems if it were run at the same time on the same computer
		// as one of your classmate's.
		// You don't need to worry about this for our class,
		// but if you ever ship a real project using this code as a base you should probably set this to something unique :)
		virtual const char* GetMainWindowClassName() const { return "Art's EAE6320 Main Window Class"; }
		// If you want you can change the icons used for your game here. The following three are provided:
		//	* IDI_EAEGAMEPAD
		//	* IDI_EAEALIEN
		//	* IDI_VSDEFAULT_LARGE / IDI_VSDEFAULT_SMALL
		// If you want to create your a convenient website that will help is: http://icoconvert.com/
		virtual const WORD* GetLargeIconId() const { static const WORD iconId_large = IDI_EAEGAMEPAD; return &iconId_large; }
		virtual const WORD* GetSmallIconId() const { static const WORD iconId_small = IDI_EAEGAMEPAD; return &iconId_small; }
#endif

		// Initialization / Clean Up
		//--------------------------

		virtual bool Initialize();
		virtual bool CleanUp();

	};
}

#endif	// CGAME_H
