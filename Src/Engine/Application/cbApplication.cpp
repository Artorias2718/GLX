// Header Files
//=============

#include "cbApplication.h"

#include <cstdlib>
#include "../Asserts/Asserts.h"
#include "../Graphics/Graphics.h"
#include "../Logging/Logging.h"
#include "../Time/Time.h"
#include "../UserOutput/UserOutput.h"

// Interface
//==========

// Run
//------

int Engine::Application::cbApplication::ParseEntryPointParametersAndRun( const sEntryPointParameters& i_entryPointParameters )
{
	int exitCode = EXIT_SUCCESS;

	// Initialize the application
	if ( Initialize_all( i_entryPointParameters ) )
	{
		Logging::OutputMessage( "The application was successfully initialized" );
	}
	else
	{
		exitCode = EXIT_FAILURE;
		ASSERT( false );
		Logging::OutputError( "Application initialization failed!" );
		UserOutput::Print( "Initialization failed! (Check the log file for details.) This program will now exit." );
		goto OnExit;
	}

	// Run the main application logic and wait for it to exit
	if ( !WaitForApplicationToFinish( exitCode ) )
	{
		ASSERT( false );
		UserOutput::Print( "The application encountered an error and will now exit" );
		goto OnExit;
	}

OnExit:

	if ( !CleanUp_all() )
	{
		ASSERT( false );
		// If any clean up fails don't overwrite any non-standard exit code that was explicitly returned by WaitForApplicationToFinish()
		if ( exitCode == EXIT_SUCCESS )
		{
			exitCode = EXIT_FAILURE;
		}
	}

	return exitCode;
}

// Initialization / Clean Up
//--------------------------

bool Engine::Application::cbApplication::Initialize_all( const sEntryPointParameters& i_entryPointParameters )
{
	// Initialize logging first so that it's always available
	if ( !Logging::Initialize( GetPathToLogTo() ) )
	{
		ASSERT( false );
		return false;
	}
	// Initialize the new application instance with entry point parameters
	if ( !Initialize_base( i_entryPointParameters ) )
	{
		ASSERT( false );
		return false;
	}
	// Initialize engine systems
	if ( !Initialize_engine() )
	{
		ASSERT( false );
		return false;	
	}
	// Initialize the game
	if ( !Initialize() )
	{
		ASSERT( false );
		return false;
	}

	return true;
}

bool Engine::Application::cbApplication::Initialize_engine()
{
	// User Output
	{
		UserOutput::sInitializationParameters initializationParameters;
		if ( PopulateUserOutputInitializationParameters( initializationParameters ) )
		{
			if ( !UserOutput::Initialize( initializationParameters ) )
			{
				ASSERT( false );
				return false;
			}
		}
		else
		{
			ASSERT( false );
			return false;
		}
	}
	// Time
	if ( !Time::Initialize() )
	{
		ASSERT( false );
		return false;
	}
	// Graphics
	{
		Graphics::sInitializationParameters initializationParameters;
		if ( PopulateGraphicsInitializationParameters( initializationParameters ) )
		{
			if ( !Graphics::Initialize( initializationParameters ) )
			{
				ASSERT( false );
				return false;
			}
		}
		else
		{
			ASSERT( false );
			return false;
		}
	}

	return true;
}

bool Engine::Application::cbApplication::CleanUp_all()
{
	bool wereThereErrors = false;

	// Clean up the game
	if ( !CleanUp() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}
	// Clean up engine systems
	if ( !CleanUp_engine() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}
	// Clean up the application
	if ( !CleanUp_base() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}
	// Clean up logging last so that messages can still be logged during clean up
	if ( !Logging::CleanUp() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}

	return !wereThereErrors;
}

bool Engine::Application::cbApplication::CleanUp_engine()
{
	bool wereThereErrors = false;

	// Graphics
	if ( !Graphics::CleanUp() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}
	// Time
	if ( !Time::CleanUp() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}
	// User Output
	if ( !UserOutput::CleanUp() )
	{
		wereThereErrors = true;
		ASSERT( false );
	}

	return !wereThereErrors;
}

Engine::Application::cbApplication::~cbApplication()
{
	CleanUp_base();
}

// Implementation
//===============

// Run
//----

void Engine::Application::cbApplication::OnNewFrame()
{
	Time::OnNewFrame();
	Graphics::RenderFrame();
}
