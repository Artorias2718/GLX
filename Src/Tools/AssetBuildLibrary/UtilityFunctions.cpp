// Header Files
//=============

#include "UtilityFunctions.h"

#include <iostream>

// Interface
//==========

void Engine::AssetBuild::OutputErrorMessage( const char* const i_errorMessage, const char* const i_optionalFileName )
{
	// This formatting causes the errors to show up in Visual Studio's "Error List" tab
	std::cerr << ( i_optionalFileName ? i_optionalFileName : "Asset Build" ) << ": error: " <<
		i_errorMessage << "\n";
}
