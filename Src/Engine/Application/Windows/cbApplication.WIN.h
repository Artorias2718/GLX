/*
	This file contains platform-specific class declarations for the application
*/

#ifndef APPLICATION_CBAPPLICATION_WIN_H
#define APPLICATION_CBAPPLICATION_WIN_H

	// Data
	//=====

private:

	HWND m_mainWindow;
	ATOM m_mainWindowClass;
	HINSTANCE m_thisInstanceOfTheApplication;

	unsigned int m_resolutionWidth, m_resolutionHeight;

	// Inheritable Implementation
	//===========================

private:

	// Configuration
	//--------------

	// Your application can override the following configuration functions

	virtual const char* GetMainWindowName() const = 0;
	virtual const char* GetMainWindowClassName() const = 0;
	virtual const WORD* GetLargeIconId() const { return NULL; }
	virtual const WORD* GetSmallIconId() const { return NULL; }


	// Utility
	//--------

protected:

	static cbApplication* GetApplicationFromWindow( const HWND i_window );

	// Implementation
	//===============

private:

	// Run
	//----

	static LRESULT CALLBACK OnMessageReceivedFromWindows( HWND i_window, UINT i_message, WPARAM i_wParam, LPARAM i_lParam );

#endif	// APPLICATION_CBAPPLICATION_WIN_H
