/*
	This file provides access to platform-specific functionality
	with a platform-independent interface
*/

#ifndef PLATFORM_H
#define PLATFORM_H

// Header Files
//=============

// Windows #defines common function names (specifically ones used here),
// and so #including windows.h first simplifies things
#ifdef WINDOWS_API
	#include "../Windows/Includes.h"
#endif

#include <cstdint>
#include <cstdlib>
#include <string>

// Interface
//==========

namespace Engine
{
	namespace Platform
	{
		struct sDataFromFile
		{
			void* data;
			size_t size;

			void Free()
			{
				if ( data != NULL )
				{
					free( data );
					data = NULL;
				}
			}

			sDataFromFile() : data( NULL ), size( 0 ) {}
		};

		bool CopyFile( const char* const i_path_source, const char* i_path_target,
			const bool i_shouldFunctionFailIfTargetAlreadyExists = false, const bool i_shouldTargetFileTimeBeModified = false,
			std::string* o_errorMessage = NULL );
		bool CreateDirectoryIfNecessary( const std::string& i_path, std::string* const o_errorMessage = NULL );
		bool DoesFileExist( const char* const i_path, std::string* const o_errorMessage = NULL );
		bool ExecuteCommand( const char* const i_command, int* const o_exitCode = NULL, std::string* const o_errorMessage = NULL );
		bool GetEnvironmentVariable( const char* const i_key, std::string& o_value, std::string* const o_errorMessage = NULL );
		bool GetLastWriteTime( const char* const i_path, uint64_t& o_lastWriteTime, std::string* const o_errorMessage = NULL );
		bool InvalidateLastWriteTime( const char* const i_path, std::string* const o_errorMessage = NULL );
		bool LoadBinaryFile( const char* const i_path, sDataFromFile& o_data, std::string* const o_errorMessage = NULL );
		// This function writes an entire file in a single operation in the most efficient way possible.
		// If you need to write out more than one smaller chunk to a file, however,
		// you should use one of the standard library functions that does buffering.
		bool WriteBinaryFile( const char* const i_path, const void* const i_data, const size_t i_size, std::string* const o_errorMessage = NULL );
	}
}

#endif	// PLATFORM_H
