// Header Files
//=============

#include "../AssertS.h"

#include "../../Windows/Includes.h"

// Helper Function Definition
//===========================

#ifdef ASSERTS_AREENABLED

bool Engine::Asserts::ShowMessageAndReturnWhetherToBreak( std::ostringstream& io_message, bool& io_shouldThisAssertBeIgnoredInTheFuture )
{
	io_message << "\n\n"
		"Do you want to break into the debugger?"
		" Choose \"Yes\" to break, \"No\" to continue, or \"Cancel\" to disable this assertion until the program exits.";
	const int result = MessageBoxA( GetActiveWindow(), io_message.str().c_str(), "Assertion Failed!", MB_YESNOCANCEL );
	if ( ( result == IDYES )
		// MessageBox() returns 0 on failure; if this happens the code breaks rather than trying to diagnose why
		|| ( result == 0 ) )
	{
		return true;
	}
	else
	{
		if ( result == IDCANCEL )
		{
			io_shouldThisAssertBeIgnoredInTheFuture = true;
		}
		return false;
	}
}

#endif	// ASSERTS_AREENABLED
