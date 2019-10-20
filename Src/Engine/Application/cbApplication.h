/*
	This is the base class for an application using this engine

	It exposes virtual platform-independent functions for the actual application
	and takes care of platform-specific application tasks.
*/

#ifndef APPLICATION_CBAPPLICATION_H
#define APPLICATION_CBAPPLICATION_H

// Header Files
//=============

#if defined( WINDOWS_API )
	#include "../Windows/Includes.h"
#endif

// Forward Declarations
//=====================

namespace Engine
{
	namespace Graphics
	{
		struct sInitializationParameters;
	}
	namespace UserOutput
	{
		struct sInitializationParameters;
	}
}

// Class Declaration
//==================

namespace Engine
{
	namespace Application
	{
		// This only thing that a specific application project's main() entry point should do
		// is to call the following function with the derived application class
		// as the template argument:
		template<class tApplication>
		int Run(
#if defined( WINDOWS_API )
			 const HINSTANCE i_thisInstanceOfTheApplication, const char* const i_commandLineArguments, const int i_initialWindowDisplayState
#endif
		)
		{
			// Create a platform-specific struct
			// that can be passed to functions in a platform-independent way
			tApplication::sEntryPointParameters entryPointParameters =
			{
#if defined( WINDOWS_API )
				i_thisInstanceOfTheApplication, i_commandLineArguments, i_initialWindowDisplayState
#endif
			};
			// Create an instance of the applciation
			tApplication newApplicationInstance;
			// Run it
			return newApplicationInstance.ParseEntryPointParametersAndRun( entryPointParameters );
		}

		class cbApplication
		{
			// Interface
			//==========

		public:

			// Different platforms have different parameters that get passed to a program's entry point
			struct sEntryPointParameters
			{
#if defined( WINDOWS_API )
				// The specific instance of the application
				// (if you have two instances of your game running simultaneously
				// this handle can differentiate between them)
				const HINSTANCE applicationInstance;
				// The arguments in the command to start the program
				const char* const commandLineArguments;
				// The requested initial state of the window
				// (e.g. minimized, maximized, etc.)
				const int initialWindowDisplayState;
#endif
			};

			// Run
			//------

			// The following function will be called from the templated Run<> function above
			// with the parameters directly from the main() entry point:
			int ParseEntryPointParametersAndRun( const sEntryPointParameters& i_entryPointParameters );

			// Info
			//-----

			bool GetResolution( unsigned int& o_width, unsigned int& o_height ) const;

			// Initialization / Clean Up
			//--------------------------

			cbApplication();
			virtual ~cbApplication() = 0;

			// Inheritable Implementation
			//===========================

		private:

			// Configuration
			//--------------

			// Your application can override the following configuration functions if desired

			virtual const char* GetPathToLogTo() const { return NULL; }	// This will be called _before_ Initialize()

			// Initialization / Clean Up
			//--------------------------

			virtual bool Initialize() = 0;
			virtual bool CleanUp() = 0;

			// Implementation
			//===============

		private:

			// Run
			//----

			void OnNewFrame();
			bool WaitForApplicationToFinish( int& o_exitCode );

			// Initialization / Clean Up
			//--------------------------

			bool Initialize_all( const sEntryPointParameters& i_entryPointParameters );	// This initializes everything
				bool Initialize_base( const sEntryPointParameters& i_entryPointParameters );	// This initializes just this base class
				bool Initialize_engine();	// This initializes all of the engine systems

			bool PopulateGraphicsInitializationParameters( Graphics::sInitializationParameters& o_initializationParameters );
			bool PopulateUserOutputInitializationParameters( UserOutput::sInitializationParameters& o_initializationParameters );

			bool CleanUp_all();	// This cleans up everything
				bool CleanUp_base();	// This cleans up just this base class
				bool CleanUp_engine();	// This cleans up all of the engine systems

			// #include the platform-specific class declarations
#if defined( WINDOWS_API )
			#include "Windows/cbApplication.win.h"
#endif
		};
	}
}

#endif	// APPLICATION_CBAPPLICATION_H
