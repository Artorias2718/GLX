// Header Files
//=============

#include "../Platform.h"

#include "../../Windows/Functions.h"

// Interface
//==========

bool Engine::Platform::CopyFile( const char* const i_path_source, const char* i_path_target,
	const bool i_shouldFunctionFailIfTargetAlreadyExists, const bool i_shouldTargetFileTimeBeModified,
	std::string* o_errorMessage )
{
	return Windows::CopyFile( i_path_source, i_path_target,
		i_shouldFunctionFailIfTargetAlreadyExists, i_shouldTargetFileTimeBeModified,
		o_errorMessage );
}

bool Engine::Platform::CreateDirectoryIfNecessary( const std::string& i_path, std::string* const o_errorMessage )
{
	return Windows::CreateDirectoryIfNecessary( i_path, o_errorMessage );
}

bool Engine::Platform::DoesFileExist( const char* const i_path, std::string* const o_errorMessage )
{
	return Windows::DoesFileExist( i_path, o_errorMessage );
}

bool Engine::Platform::ExecuteCommand( const char* const i_command, int* const o_exitCode, std::string* const o_errorMessage )
{
	DWORD exitCode_unsigned;
	const bool result = Windows::ExecuteCommand( i_command, &exitCode_unsigned, o_errorMessage );
	if ( o_exitCode )
	{
		int32_t exitCode_signed = static_cast<int32_t>( exitCode_unsigned );
		*o_exitCode = static_cast<int>( exitCode_signed );
	}
	return result;
}

bool Engine::Platform::GetEnvironmentVariable( const char* const i_key, std::string& o_value, std::string* const o_errorMessage )
{
	return Windows::GetEnvironmentVariable( i_key, o_value, o_errorMessage );
}

bool Engine::Platform::GetLastWriteTime( const char* const i_path, uint64_t& o_lastWriteTime, std::string* const o_errorMessage )
{
	return Windows::GetLastWriteTime( i_path, o_lastWriteTime, o_errorMessage );
}

bool Engine::Platform::InvalidateLastWriteTime( const char* const i_path, std::string* const o_errorMessage )
{
	return Windows::InvalidateLastWriteTime( i_path, o_errorMessage );
}

bool Engine::Platform::LoadBinaryFile( const char* const i_path, sDataFromFile& o_data, std::string* const o_errorMessage )
{
	Windows::sDataFromFile dataFromFile;
	const bool result = Windows::LoadBinaryFile( i_path, dataFromFile, o_errorMessage );
	{
		o_data.data = dataFromFile.data;
		o_data.size = dataFromFile.size;
	}
	return result;
}

bool Engine::Platform::WriteBinaryFile( const char* const i_path, const void* const i_data, const size_t i_size, std::string* const o_errorMessage )
{
	return Windows::WriteBinaryFile( i_path, i_data, i_size, o_errorMessage );
}