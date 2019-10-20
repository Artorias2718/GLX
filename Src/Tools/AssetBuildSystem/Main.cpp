/*
	The main() function is where the program starts execution
*/

// Header Files
//=============

#include <cstdlib>
#include <iostream>
#include "../AssetBuildLibrary/UtilityFunctions.h"
#include "../../Engine/Platform/Platform.h"

// Static Data Initialization
//===========================

namespace
{
	std::string s_AuthoredAssetDir;
	std::string s_BuiltAssetDir;
}

// Helper Function Declarations
//=============================

namespace
{
	bool Initialize();
	bool BuildAsset( const char* const i_relativePath );
}

// Entry Point
//============

int main( int i_argumentCount, char** i_arguments )
{
	if ( !Initialize() )
	{
		return EXIT_FAILURE;
	}

	bool wereThereErrors = false;

	// The command line should have a list of assets to build
	for ( int i = 1; i < i_argumentCount; ++i )
	{
		const char* relativePath = i_arguments[i];
		if ( !BuildAsset( relativePath ) )
		{
			wereThereErrors = true;
		}
	}

	if ( !wereThereErrors )
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}

// Helper Function Definitions
//============================

namespace
{
	bool Initialize()
	{
		// These environment variables are set in SolutionMacros.props
		if ( !Engine::Platform::GetEnvironmentVariable( "AuthoredAssetDir", s_AuthoredAssetDir ) )
		{
			return false;
		}
		if ( !Engine::Platform::GetEnvironmentVariable( "BuiltAssetDir", s_BuiltAssetDir ) )
		{
			return false;
		}

		return true;
	}

	bool BuildAsset( const char* const i_relativePath )
	{
		// Get the absolute paths to the source and target
		// (The "source" is the authored asset,
		// and the "target" is the built asset that is ready to be used in-game.
		// In this example program we will just copy the source to the target
		// and so the two will be the same,
		// but in a real asset build pipeline the two will usually be different:
		// The source will be in a format that is optimal for authoring purposes
		// and the target will be in a format that is optimal for real-time purposes.)
		const std::string path_source = s_AuthoredAssetDir + i_relativePath;
		const std::string path_target = s_BuiltAssetDir + i_relativePath;

		// If the source file doesn't exist then it can't be built
		{
			std::string errorMessage;
			if ( !Engine::Platform::DoesFileExist( path_source.c_str(), &errorMessage ) )
			{
				Engine::AssetBuild::OutputErrorMessage( errorMessage.c_str(), path_source.c_str() );
				return false;
			}
		}

		// Decide if the target needs to be built
		bool shouldTargetBeBuilt;
		{
			// The simplest reason a target should be built is if it doesn't exist
			if ( Engine::Platform::DoesFileExist( path_target.c_str() ) )
			{
				// Even if the target exists it may be out-of-date.
				// If the source has been modified more recently than the target
				// then the target should be re-built.
				uint64_t lastWriteTime_source, lastWriteTime_target;
				{
					std::string errorMessage;
					if ( !Engine::Platform::GetLastWriteTime( path_source.c_str(), lastWriteTime_source, &errorMessage ) ||
						!Engine::Platform::GetLastWriteTime( path_target.c_str(), lastWriteTime_target, &errorMessage ) )
					{
						Engine::AssetBuild::OutputErrorMessage( errorMessage.c_str() );
						return false;
					}
				}
				shouldTargetBeBuilt = lastWriteTime_source > lastWriteTime_target;
			}
			else
			{
				shouldTargetBeBuilt = true;
			}
		}

		// Build the target if necessary
		if ( shouldTargetBeBuilt )
		{
			std::string errorMessage;

			// Display a message to the user for each asset
			std::cout << "Building " << path_source << "\n";

			// Create the target directory if necessary
			if ( !Engine::Platform::CreateDirectoryIfNecessary( path_target, &errorMessage ) )
			{
				Engine::AssetBuild::OutputErrorMessage( errorMessage.c_str(), path_target.c_str() );
				return false;
			}

			// Copy the source to the target
			{
				// There are many reasons that a source should be rebuilt,
				// and so even if the target already exists it should just be written over
				const bool shouldFunctionFailIfTargetAlreadyExists = false;
				// Since we rely on timestamps to determine when a target was built
				// its file time should be updated when the source gets copied
				const bool shouldTargetFileTimeBeModified = true;
				if ( !Engine::Platform::CopyFile( path_source.c_str(), path_target.c_str(),
					shouldFunctionFailIfTargetAlreadyExists,shouldTargetFileTimeBeModified,
					&errorMessage ) )
				{
					Engine::AssetBuild::OutputErrorMessage( errorMessage.c_str(), path_target.c_str() );
					return false;
				}
			}
		}

		return true;
	}
}
