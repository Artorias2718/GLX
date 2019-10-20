// Header Files
//=============

#include "../cbApplication.h"

#include <cstdlib>
#include "../../Asserts/Asserts.h"
#include "../../Graphics/Graphics.h"
#include "../../Logging/Logging.h"
#include "../../UserOutput/UserOutput.h"
#include "../../UserSettings/UserSettings.h"
#include "../../Windows/Functions.h"

// Helper Function Declarations
//=============================

namespace
{
	// Initialization / Clean Up
	//--------------------------

	bool CreateMainWindow( const HINSTANCE i_thisInstanceOfTheApplication, const char* const i_windowName, const ATOM i_windowClass,
		const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight,
		Engine::Application::cbApplication& io_application, HWND& o_window );
	bool CreateMainWindowClass( const HINSTANCE i_thisInstanceOfTheApplication, const char* const i_mainWindowClassName,
		WNDPROC fOnMessageReceivedFromWindows, ATOM& o_mainWindowClass,
		const WORD* const i_iconId_large = NULL, const WORD* const i_iconId_small = NULL );
	bool FreeMainWindow( HWND& io_window );
	bool FreeMainWindowClass( const HINSTANCE i_thisInstanceOfTheApplication, ATOM& io_mainWindowClass );
}

// Interface
//==========

// Info
//-----

bool Engine::Application::cbApplication::GetResolution( unsigned int& o_width, unsigned int& o_height ) const
{
	if ( ( m_resolutionWidth != 0 ) && ( m_resolutionHeight != 0 ) )
	{
		o_width = m_resolutionWidth;
		o_height = m_resolutionHeight;
		return true;
	}
	else
	{
		return false;
	}
}

// Inheritable Implementation
//===========================

// Utility
//--------

Engine::Application::cbApplication* Engine::Application::cbApplication::GetApplicationFromWindow( const HWND i_window )
{
	const LONG_PTR userData = GetWindowLongPtr( i_window, GWLP_USERDATA );
	cbApplication* const application = reinterpret_cast<cbApplication*>( userData );
	ASSERT( application != NULL );
	ASSERT( application->m_mainWindow == i_window );
	return application;
}

// Implementation
//===============

// Run
//----

bool Engine::Application::cbApplication::WaitForApplicationToFinish( int& o_exitCode )
{
	// Enter an infinite loop that will continue until a WM_QUIT message is received from Windows
	MSG message = { 0 };
	do
	{
		// To send us a message, Windows will add it to a queue.
		// Most Windows applications should wait until a message is received and then react to it.
		// Real-time programs, though, must continually draw new images to the screen as fast as possible
		// and only pause momentarily when there is a Windows message to deal with.

		// This means that the first thing that must be done every iteration of the game loop is to "peek" at the message queue
		// and see if there are any messages from Windows that need to be handled
		bool hasWindowsSentAMessage;
		{
			HWND getMessagesFromAnyWindowBelongingToTheCurrentThread = NULL;
			unsigned int getAllMessageTypes = 0;
			unsigned int ifAMessageExistsRemoveItFromTheQueue = PM_REMOVE;
			hasWindowsSentAMessage = PeekMessage( &message, getMessagesFromAnyWindowBelongingToTheCurrentThread,
				getAllMessageTypes, getAllMessageTypes, ifAMessageExistsRemoveItFromTheQueue ) == TRUE;
		}
		if ( !hasWindowsSentAMessage )
		{
			// Usually there will be no messages in the queue, and the game can run
			OnNewFrame();
		}
		else
		{
			// If Windows _has_ sent a message, this iteration of the loop will handle it

			// First, the message must be "translated"
			// (Key presses are translated into character messages)
			TranslateMessage( &message );

			// Then, the message is sent on to the appropriate processing function.
			// This function is specified in the lpfnWndProc field of the WNDCLASSEX struct
			// used to register a class with Windows.
			// In the case of the main window in this example program
			// it will always be OnMessageReceived()
			DispatchMessage( &message );
		}
	} while ( message.message != WM_QUIT );

	// The exit code for the application is stored in the WPARAM of a WM_QUIT message
	o_exitCode = static_cast<int>( message.wParam );

	return true;
}

LRESULT CALLBACK Engine::Application::cbApplication::OnMessageReceivedFromWindows( HWND i_window, UINT i_message, WPARAM i_wParam, LPARAM i_lParam )
{
	// DispatchMessage() will send messages that the main window receives to this function.
	// There are many messages that get sent to a window,
	// but your game can ignore most of them
	// and let Windows handle them in the default way.

	// Process any messages that the game cares about
	// (any messages that are processed here should return a value
	// rather than letting the default processing function try to handle them a second time)
	switch( i_message )
	{
	// A key has been pressed down, and this is the translated character
	case WM_CHAR:
		{
			// Allow the user to quit if s/he presses the escape key.
			// This is platform-specific, and even for Windows this is not the best way to handle realt-time input,
			// but it is convenient for the sake of simplicity in our class.
			// (The WPARAM input parameter indicates which key was pressed,
			// but this example program only cares about the escape key)
			if ( i_wParam == VK_ESCAPE )
			{
				int result;
				{
					const char* const caption = "Exit Application?";
					const char* const message = "Are you sure you want to quit?";
					result = MessageBox( i_window, message, caption, MB_YESNO | MB_ICONQUESTION );
				}
				if ( result == IDYES )
				{
					// Instruct Windows to send a WM_QUIT message
					{
						// You can send any exit code that you want.
						// You could, for example, send a different exit code
						// to indicate whether the program exited because someone pressed the escape key (here)
						// or because the window was closed (see WM_NCDESTROY below).
						// In our class we don't care about anything fancy,
						// and the generic success code is a good choice.
						const int exitCode = EXIT_SUCCESS;
						PostQuitMessage( exitCode );	// This sends a WM_QUIT message
					}

					// Return a value to indicate that we processed this message
					// (the correct value is different for different messages types;
					// for WM_CHAR it is 0)
					return 0;
				}
				else
				{
					// If the user doesn't select "Yes" to quit
					// then do nothing to keep running the program
				}
			}
		}
		break;
	// A window has been created
	case WM_CREATE:
		{
			// This message gets sent as a result of CreateWindowEx() being called
			// and is received here before that function returns

			// The LPARAM input parameter has information about the window's creation
			const CREATESTRUCT& creationData = *reinterpret_cast<CREATESTRUCT*>( i_lParam );
			// We are interested specifically in the application pointer that we gave to CreateWindowEx()
			Engine::Application::cbApplication* application =
				reinterpret_cast<Engine::Application::cbApplication*>( creationData.lpCreateParams );
			ASSERT( application != NULL );
			// Assign the new handle
			application->m_mainWindow = i_window;
			// Assign the application to the new window's user data
			{
				SetLastError( ERROR_SUCCESS );
				const LONG_PTR previousUserData = SetWindowLongPtr( i_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( application ) );
				// Presumably there is no previous user data on a brand new window
				if ( previousUserData == NULL )
				{
					// Make sure that there was no error
					DWORD errorCode;
					const std::string errorMessage = Windows::GetLastSystemError( &errorCode );
					if ( errorCode != ERROR_SUCCESS )
					{
						ASSERTF( "Couldn't set main window user data: %s", errorMessage.c_str() );
						Logging::OutputError( "Windows failed to set the main window's user data: %s", errorMessage.c_str() );
						// Returning -1 causes the window to be destroyed and a NULL window handle to be returned from CreateWindowEx()
						application->m_mainWindow = NULL;
						return -1;
					}
				}
			}

			// Return a value to indicate that we processed this message
			// (the correct value is different for different messages types;
			// for WM_CREATE it is 0 on success)
			return 0;
		}
		break;
	// A window's nonclient area is being destroyed
	// (because e.g. someone has clicked the X in upper-right corner)
	case WM_NCDESTROY:
		{
			// This is the last message a window will receive
			// (any child windows have already been destroyed).
			// After this message has been processed the window's handle will be invalid,
			// so unassign it from the application
			{
				cbApplication* const application = GetApplicationFromWindow( i_window );
				if ( application != NULL )
				{
					application->m_mainWindow = NULL;
				}
			}

			// When the main window is destroyed
			// a WM_QUIT message should be sent
			// (if this isn't done the application would continue to run with no window).
			// You can send any exit code that you want,
			// but the default "success" is probably a good choice
			// (it is actually quite unlikely that anything
			// will ever pay attention to or care about your game's exit code)
			const int exitCode = EXIT_SUCCESS;
			PostQuitMessage( exitCode );	// This sends a WM_QUIT message

			// Return a value to indicate that we processed this message
			// (the correct value is different for different messages types;
			// for WM_NCDESTROY it is 0)
			return 0;
		}
		break;
	}

	// Pass any messages that weren't handled on to Windows
	// so that our window will behave like any standard window
	return DefWindowProc( i_window, i_message, i_wParam, i_lParam );
}

// Initialization / Clean Up
//--------------------------

bool Engine::Application::cbApplication::Initialize_base( const sEntryPointParameters& i_entryPointParameters )
{
	bool wereThereErrors = false;

	// Save the handle to this specific running instance of the application
	m_thisInstanceOfTheApplication = i_entryPointParameters.applicationInstance;

	// Windows requires every window to have a "class"
	// (a window only has a single class, but many windows can share the same class)
	if ( !CreateMainWindowClass( m_thisInstanceOfTheApplication, GetMainWindowClassName(),
		OnMessageReceivedFromWindows, m_mainWindowClass,
		GetLargeIconId(), GetSmallIconId() ) )
	{
		ASSERT( false );
		wereThereErrors = true;
		goto OnExit;
	}
	// Create the main window
	{
		HWND hWindow;
		const unsigned int desiredResolutionWidth = UserSettings::GetResolutionWidth();
		const unsigned int desiredResolutionHeight = UserSettings::GetResolutionHeight();
		if ( CreateMainWindow( m_thisInstanceOfTheApplication, GetMainWindowName(), m_mainWindowClass,
			desiredResolutionWidth, desiredResolutionHeight,
			*this, hWindow ) )
		{
			// The new window handle should have been assigned to the application in OnMessageReceivedFromWindows() WM_CREATE
			ASSERT( hWindow == m_mainWindow );
			// Assign the resolution
			// (the window's border can't be resized, so for our class it's ok to set the resolution once and leave it)
			m_resolutionWidth = desiredResolutionWidth;
			m_resolutionHeight = desiredResolutionHeight;
		}
		else
		{
			ASSERT( false );
			wereThereErrors = true;
			goto OnExit;
		}
	}
	// Display the window in the initial state that Windows requested
	{
		const BOOL wasWindowPreviouslyVisible = ShowWindow( m_mainWindow, i_entryPointParameters.initialWindowDisplayState );
	}

OnExit:

	return !wereThereErrors;
}

bool Engine::Application::cbApplication::PopulateGraphicsInitializationParameters( Graphics::sInitializationParameters& o_initializationParameters )
{
	ASSERT( m_mainWindow != NULL );
	o_initializationParameters.mainWindow = m_mainWindow;
#if defined( D3D_API )
	o_initializationParameters.resolutionWidth = m_resolutionWidth;
	o_initializationParameters.resolutionHeight = m_resolutionHeight;
#elif defined( GL_API )
	o_initializationParameters.thisInstanceOfTheApplication = m_thisInstanceOfTheApplication;
#endif
	return true;
}

bool Engine::Application::cbApplication::PopulateUserOutputInitializationParameters( UserOutput::sInitializationParameters& o_initializationParameters )
{
	ASSERT( m_mainWindow != NULL );
	o_initializationParameters.mainWindow = m_mainWindow;
	return true;
}

bool Engine::Application::cbApplication::CleanUp_base()
{
	bool wereThereErrors = false;

	// Main Window
	if ( m_mainWindow )
	{
		if ( !FreeMainWindow( m_mainWindow ) )
		{
			ASSERT( false );
			wereThereErrors = true;
		}
	}
	// Main Window Class
	if ( m_mainWindowClass )
	{
		if ( !FreeMainWindowClass( m_thisInstanceOfTheApplication, m_mainWindowClass ) )
		{
			ASSERT( false );
			wereThereErrors = true;
		}
	}

	return !wereThereErrors;
}

Engine::Application::cbApplication::cbApplication()
	:
	m_mainWindow( NULL ), m_mainWindowClass( NULL ), m_thisInstanceOfTheApplication( NULL ),
	m_resolutionWidth( 0 ), m_resolutionHeight( 0 )
{

}

// Helper Function Definitions
//============================

namespace
{
	// Initialization / Clean Up
	//--------------------------

	bool CreateMainWindow( const HINSTANCE i_thisInstanceOfTheApplication, const char* const i_windowName, const ATOM i_windowClass,
		const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight,
		Engine::Application::cbApplication& io_application, HWND& o_window )
	{
		// Create the window
		{
			// The window's style
			const DWORD windowStyle =
				// "Overlapped" is basically the same as "top-level"
				WS_OVERLAPPED
				// The caption is the title bar when in windowed-mode
				| WS_CAPTION
				// The window should never change dimensions, so only a minimize box is allowed
				| WS_MINIMIZEBOX
				// The system menu appears when you right-click the title bar
				| WS_SYSMENU;
			// The window's extended style
			const DWORD windowStyle_extended =
				// The following is a macro to make the extended style the default top-level look
				WS_EX_OVERLAPPEDWINDOW;
			// The width and height of the window.
			// A game cares about the width and height of the actual "client area",
			// which is the part of the window that doesn't include the borders and title bar;
			// this means that if we say that a game runs at a resolution of 800 x 600,
			// the actual window will be slightly bigger than that.
			// Initially, then, the window will be created with default values that Windows chooses
			// and then resized after creation)
			const int width = CW_USEDEFAULT;
			const int height = CW_USEDEFAULT;
			// The initial position of the window
			// (We don't care, and will let Windows decide)
			const int position_x = CW_USEDEFAULT;
			const int position_y = CW_USEDEFAULT;
			// Handle to the parent of this window
			// (Since this is our main window, it can't have a parent)
			const HWND hParent = NULL;
			// Handle to the menu for this window
			// (The main window won't have a menu)
			const HMENU hMenu = NULL;
			// A pointer can be sent with the WM_CREATE message received in OnMessageReceivedFromWindows()
			void* const userData = &io_application;

			// Ask Windows to create the specified window.
			// CreateWindowEx() will return a "handle" to the window (kind of like a pointer),
			// which is what we'll use when communicating with Windows to refer to this window
			o_window = CreateWindowEx( windowStyle_extended, MAKEINTATOM( i_windowClass ), i_windowName, windowStyle,
				position_x, position_y, width, height,
				hParent, hMenu, i_thisInstanceOfTheApplication, userData );
			if ( o_window != NULL )
			{
				Engine::Logging::OutputMessage( "Created main window \"%s\"", i_windowName );
			}
			else
			{
				const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
				ASSERTF( false, "Main window wasn't created: %s", windowsErrorMessage.c_str() );
				Engine::Logging::OutputError( "Windows failed to create the main window: %s", windowsErrorMessage.c_str() );
				return false;
			}
		}

		// Change the window's size based on the desired client area resolution
		{
			// Calculate how much of the window is coming from the "non-client area"
			// (the borders and title bar)
			RECT windowCoordinates;
			struct
			{
				long width;
				long height;
			} nonClientAreaSize;
			{
				// Get the coordinates of the entire window
				if ( GetWindowRect( o_window, &windowCoordinates ) == FALSE )
				{
					const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
					ASSERTF( false, "Couldn't get coordinates of the main window: %s", windowsErrorMessage.c_str() );
					Engine::Logging::OutputError( "Windows failed to get the coordinates of the main window: %s", windowsErrorMessage.c_str() );
					goto OnError;
				}
				// Get the dimensions of the client area
				RECT clientDimensions;
				if ( GetClientRect( o_window, &clientDimensions ) == FALSE )
				{
					const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
					ASSERTF( false, "Couldn't get the main window's client dimensions: %s", windowsErrorMessage.c_str() );
					Engine::Logging::OutputError( "Windows failed to get the dimensions of the main window's client area:: %s",
						windowsErrorMessage.c_str() );
					goto OnError;
				}
				// Get the difference between them
				nonClientAreaSize.width = ( windowCoordinates.right - windowCoordinates.left ) - clientDimensions.right;
				nonClientAreaSize.height = ( windowCoordinates.bottom - windowCoordinates.top ) - clientDimensions.bottom;
			}
			// Resize the window
			{
				const int desiredWidth_window = i_resolutionWidth + nonClientAreaSize.width;
				const int desiredHeight_window = i_resolutionHeight + nonClientAreaSize.height;
				const BOOL redrawTheWindowAfterItsBeenResized = TRUE;
				if ( MoveWindow( o_window,
					windowCoordinates.left, windowCoordinates.top, desiredWidth_window, desiredHeight_window,
					redrawTheWindowAfterItsBeenResized ) != FALSE )
				{
					Engine::Logging::OutputMessage( "Set main window resolution to %u x %u",
						i_resolutionWidth, i_resolutionHeight );
				}
				else
				{
					const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
					ASSERTF( false, "Couldn't resize the main window to &i x &i: %s",
						desiredWidth_window, desiredHeight_window, windowsErrorMessage.c_str() );
					Engine::Logging::OutputError( "Windows failed to resize main window to &i x &i"
						" (based on a desired resolution of %u x %u): %s",
						desiredWidth_window, desiredHeight_window, i_resolutionWidth, i_resolutionHeight, windowsErrorMessage.c_str() );
					goto OnError;
				}
			}
		}

		return true;

	OnError:

		if ( o_window != NULL )
		{
			FreeMainWindow( o_window );
		}

		return false;
	}

	bool CreateMainWindowClass( const HINSTANCE i_thisInstanceOfTheApplication, const char* const i_mainWindowClassName,
		WNDPROC fOnMessageReceivedFromWindows, ATOM& o_mainWindowClass,
		const WORD* const i_iconId_large, const WORD* const i_iconId_small )
	{
		// Populate a struct defining the window class that we want
		WNDCLASSEX wndClassEx = { 0 };
		{
			wndClassEx.cbSize = sizeof( WNDCLASSEX );
			wndClassEx.hInstance = i_thisInstanceOfTheApplication;

			// The class's style
			// (We don't have to worry about any of these)
			wndClassEx.style = 0;
			// The function that will process all of the messages
			// that Windows will send to windows of this class
			wndClassEx.lpfnWndProc = fOnMessageReceivedFromWindows;
			// Extra bytes can be set aside in the class for user data
			wndClassEx.cbClsExtra = 0;
			// Extra bytes can be set aside for each window of this class,
			// but this is usually specified for each window individually
			wndClassEx.cbWndExtra = 0;
			// The large and small icons that windows of this class should use
			if ( i_iconId_large )
			{
				wndClassEx.hIcon = LoadIcon( i_thisInstanceOfTheApplication, MAKEINTRESOURCE( *i_iconId_large ) );
			}
			else
			{
				wndClassEx.hIcon = LoadIcon( NULL, IDI_APPLICATION );
			}
			if ( i_iconId_small != 0 )
			{
				wndClassEx.hIconSm = LoadIcon( i_thisInstanceOfTheApplication, MAKEINTRESOURCE( *i_iconId_small ) );
			}
			else
			{
				wndClassEx.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
			}
			// The cursor that should display when the mouse pointer is over windows of this class
			// (in a real game you would likely hide this and render your own)
			wndClassEx.hCursor = LoadCursor( NULL, IDC_ARROW );
			// The "brush" that windows of this class should use as a background
			// (Setting this is a bit confusing but not important,
			// so don't be alarmed if the next line looks scary,
			// especially since 1) our entire window will be filled by the rendering and we won't see background
			// and 2) the technique below doesn't seem to work anymore since Windows Vista)
			wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>( IntToPtr( COLOR_BACKGROUND + 1 ) );
			// A menu can be specified that all windows of this class would use by default,
			// but usually this is set for each window individually
			wndClassEx.lpszMenuName = NULL;
			// The class name (see comments where this is initialized)
			wndClassEx.lpszClassName = i_mainWindowClassName;
		}
		// Register the class with Windows
		o_mainWindowClass = RegisterClassEx( &wndClassEx );
		if ( o_mainWindowClass != NULL )
		{
			Engine::Logging::OutputMessage( "Registered main window class \"%s\"", i_mainWindowClassName );
			return true;
		}
		else
		{
			const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
			ASSERTF( false, "Main window class registration failed: %s", windowsErrorMessage.c_str() );
			Engine::Logging::OutputError( "Windows failed to register the main window class: %s", windowsErrorMessage.c_str() );
			return false;
		}
	}

	bool FreeMainWindow( HWND& io_window )
	{
		if ( DestroyWindow( io_window ) != FALSE )
		{
			Engine::Logging::OutputMessage( "Destroyed main window" );
			// The handle should have been set to NULL in OnMessageReceivedFromWindows() WM_NCDESTROY
			ASSERT( io_window == NULL );
			io_window = NULL;
			return true;
		}
		else
		{
			const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
			ASSERTF( false, "Main window wasn't destroyed: %s", windowsErrorMessage.c_str() );
			Engine::Logging::OutputError( "Windows failed to destroy the main window: %s", windowsErrorMessage.c_str() );
			return false;
		}
	}

	bool FreeMainWindowClass( const HINSTANCE i_thisInstanceOfTheApplication, ATOM& io_mainWindowClass  )
	{
		if ( UnregisterClass( MAKEINTATOM( io_mainWindowClass ), i_thisInstanceOfTheApplication ) != FALSE )
		{
			Engine::Logging::OutputMessage( "Unregistered main window class" );
			io_mainWindowClass = NULL;
			return true;
		}
		else
		{
			const std::string windowsErrorMessage = Engine::Windows::GetLastSystemError();
			ASSERTF( false, "Main window class wasn't unregistered: %s", windowsErrorMessage.c_str() );
			Engine::Logging::OutputError( "Windows failed to unregister the main window class: %s", windowsErrorMessage.c_str() );
			return false;
		}
	}
}
