/*
	This file contains utility functions for other asset build tools
*/

#ifndef ASSETBUILD_UTILITYFUNCTIONS_H
#define ASSETBUILD_UTILITYFUNCTIONS_H

#ifndef NULL
	#define NULL 0
#endif

// Interface
//==========

namespace Engine
{
	namespace AssetBuild
	{
		// Errors can be formatted a specific way so that they show up
		// in Visual Studio's "Error List" tab
		void OutputErrorMessage( const char* const i_errorMessage, const char* const i_optionalFileName = NULL );
	}
}

#endif	// ASSETBUILD_UTILITYFUNCTIONS_H
