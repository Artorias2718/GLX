// Header Files
//=============

#include "Functions.h"

#include <iostream>
#include <regex>
#include <ShlObj.h>
#include <sstream>
#include "../Asserts/Asserts.h"

// Interface
//==========

bool Engine::Windows::CopyFile( const char* const i_path_source, const char* const i_path_target,
	const bool i_shouldFunctionFailIfTargetAlreadyExists, const bool i_shouldTargetFileTimeBeModified,
	std::string* const o_errorMessage )
{
	if ( ::CopyFile( i_path_source, i_path_target, i_shouldFunctionFailIfTargetAlreadyExists ) != FALSE )
	{
		if ( i_shouldTargetFileTimeBeModified )
		{
			// Get the current system time
			FILETIME fileTime;
			{
				SYSTEMTIME systemTime;
				GetSystemTime( &systemTime );
				if ( SystemTimeToFileTime( &systemTime, &fileTime ) == FALSE )
				{
					if ( o_errorMessage )
					{
						*o_errorMessage = GetLastSystemError();
					}
					return false;
				}
			}
			// Set the current system time as the file's last write time
			{
				bool wereThereErrors = false;

				HANDLE fileHandle = INVALID_HANDLE_VALUE;
				{
					const DWORD desiredAccess = FILE_WRITE_ATTRIBUTES;
					const DWORD otherProgramsCanStillReadTheFile = FILE_SHARE_READ;
					SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
					const DWORD onlySucceedIfFileExists = OPEN_EXISTING;
					const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
					const HANDLE dontUseTemplateFile = NULL;
					fileHandle = CreateFile( i_path_target, desiredAccess, otherProgramsCanStillReadTheFile,
						useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile );
					if ( fileHandle == INVALID_HANDLE_VALUE )
					{
						wereThereErrors = true;
						if ( o_errorMessage )
						{
							*o_errorMessage = GetLastSystemError();
						}
						goto OnDoneChangingTime;
					}
				}
				{
					FILETIME* const onlyChangeLastWriteTime = NULL;
					if ( SetFileTime( fileHandle, onlyChangeLastWriteTime, onlyChangeLastWriteTime, &fileTime ) == FALSE )
					{
						wereThereErrors = true;
						if ( o_errorMessage )
						{
							*o_errorMessage = GetLastSystemError();
						}
						goto OnDoneChangingTime;
					}
				}

			OnDoneChangingTime:

				if ( fileHandle != INVALID_HANDLE_VALUE )
				{
					if ( CloseHandle( fileHandle ) == FALSE )
					{
						wereThereErrors = true;
						const std::string windowsErrorMessage = GetLastSystemError();
						if ( o_errorMessage )
						{
							*o_errorMessage += "\n";
							*o_errorMessage += windowsErrorMessage;
						}
					}
					fileHandle = INVALID_HANDLE_VALUE;
				}
				if ( wereThereErrors )
				{
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		if ( o_errorMessage )
		{
			*o_errorMessage = GetLastSystemError();
		}
		return false;
	}
}

bool Engine::Windows::CreateDirectoryIfNecessary( const std::string& i_path, std::string* const o_errorMessage )
{
	// If the path is to a file (likely), remove it so that only the directory remains
	std::string directory;
	{
		size_t pos_slash = i_path.find_last_of( "\\/" );
		if ( pos_slash != std::string::npos )
		{
			directory = i_path.substr( 0, pos_slash );
		}
		else
		{
			directory = i_path;
		}
	}
	// Get the path in a form Windows likes (without any ".."s).
	// Windows requires a character buffer
	// to copy the path variable into.
	const DWORD maxCharacterCount = MAX_PATH;
	static char buffer[maxCharacterCount];
	{
		char** pathIsDirectory = NULL;
		DWORD characterCount = GetFullPathName( directory.c_str(), maxCharacterCount, buffer, pathIsDirectory );
		if ( characterCount > 0 )
		{
			if ( characterCount <= maxCharacterCount )
			{
				// Create the directory
				int result;
				{
					HWND noWindowIsDisplayed = NULL;
					const SECURITY_ATTRIBUTES* useDefaultSecurityAttributes = NULL;
					result = SHCreateDirectoryEx( noWindowIsDisplayed, buffer, useDefaultSecurityAttributes );
				}
				if ( ( result == ERROR_SUCCESS ) || ( ( result == ERROR_FILE_EXISTS ) || ( result== ERROR_ALREADY_EXISTS ) ) )
				{
					return true;
				}
				else
				{
					if ( o_errorMessage )
					{
						*o_errorMessage = GetFormattedSystemMessage( result );
					}
					return false;
				}
			}
			else
			{
				// If you're seeing this error you will need to increase maxCharacterCount
				if ( o_errorMessage )
				{
					std::ostringstream errorMessage;
					errorMessage << "The full path of \"" << directory << "\" requires " << characterCount <<
						" characters and the provided buffer only has room for " << maxCharacterCount;
					*o_errorMessage = errorMessage.str();
				}
				return false;
			}
		}
		else
		{
			if ( o_errorMessage )
			{
				*o_errorMessage = GetLastSystemError();
			}
			return false;
		}
	}
}

bool Engine::Windows::DoesFileExist( const char* const i_path, std::string* const o_errorMessage )
{
	// Try to get information about the file
	WIN32_FIND_DATA fileData;
	HANDLE file = FindFirstFile( i_path, &fileData );
	if ( file != INVALID_HANDLE_VALUE )
	{
		if ( FindClose( file ) == FALSE )
		{
			const std::string windowsErrorMessage = GetLastSystemError();
			ASSERTF( false, "Windows failed to close the file handle to \"%s\" after finding it: %s",
				i_path, windowsErrorMessage.c_str() );
			if ( o_errorMessage )
			{
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to close the file handle to \"" << i_path << "\" after finding it: " << windowsErrorMessage;
				*o_errorMessage = errorMessage.str();
			}
		}
		return true;
	}
	else
	{
		DWORD errorCode;
		std::string errorMessage = GetLastSystemError( &errorCode );
		ASSERTF( ( ( errorCode == ERROR_FILE_NOT_FOUND ) || ( errorCode == ERROR_PATH_NOT_FOUND ) ),
			"FindFirstFile() failed with the unexpected error code of %u: %s", errorCode, errorMessage.c_str() );
		if ( o_errorMessage )
		{
			*o_errorMessage = errorMessage;
		}
		return false;
	}
}

bool Engine::Windows::ExecuteCommand( const char* const i_command, DWORD* const o_exitCode, std::string* const o_errorMessage )
{
	return ExecuteCommand( NULL, i_command, o_exitCode, o_errorMessage );
}

bool Engine::Windows::ExecuteCommand( const char* const i_path, const char* const i_optionalArguments,
	DWORD* const o_exitCode, std::string* const o_errorMessage )
{
	bool wereThereErrors = false;

	// Get a non-const char* command line
	std::string path;
	const DWORD argumentBufferSize = 1024;
	static char arguments[argumentBufferSize];
	{
		std::string optionalArguments( i_optionalArguments );
		if ( i_path )
		{
			path = i_path;
		}
		else
		{
			// If the path is part of the optional arguments then separate it
			const size_t pos_firstNonSpace = optionalArguments.find_first_not_of( " \t" );
			const size_t pos_firstNonQuote = optionalArguments.find_first_not_of( '\"', pos_firstNonSpace );
			const size_t quoteCountBeginning = pos_firstNonQuote - pos_firstNonSpace;
			if ( quoteCountBeginning == 0 )
			{
				// If there are no quotes then the command ends at the first space
				const size_t pos_firstSpaceAfterCommand = optionalArguments.find_first_of( " \t", pos_firstNonQuote );
				path = optionalArguments.substr( pos_firstNonQuote, pos_firstSpaceAfterCommand - pos_firstNonQuote );
				const size_t pos_firstNonSpaceAfterCommand = optionalArguments.find_first_not_of( " \t", pos_firstSpaceAfterCommand );
				optionalArguments = optionalArguments.substr( pos_firstNonSpaceAfterCommand );
			}
			else
			{
				// If there are quotes then the command ends at the next quote
				const size_t pos_firstQuoteAfterCommand = optionalArguments.find_first_of( '\"', pos_firstNonQuote );
				path = optionalArguments.substr( pos_firstNonQuote, pos_firstQuoteAfterCommand - pos_firstNonQuote );
				const size_t pos_firstNonQuoteAfterCommand = optionalArguments.find_first_not_of( '\"', pos_firstQuoteAfterCommand );
				const size_t quoteCountAfterCommand = pos_firstNonQuoteAfterCommand - pos_firstQuoteAfterCommand;
				const size_t pos_firstNonSpaceAfterCommand = optionalArguments.find_first_not_of( " \t", pos_firstNonQuoteAfterCommand );
				optionalArguments = optionalArguments.substr( pos_firstNonSpaceAfterCommand );
				// If the entire command line was surrounded by quotes the trailing ones must be removed
				if ( quoteCountAfterCommand < quoteCountBeginning )
				{
					for ( size_t quoteCountToRemoveFromEnd =  quoteCountBeginning - quoteCountAfterCommand;
						quoteCountToRemoveFromEnd > 0; --quoteCountToRemoveFromEnd )
					{
						const size_t pos_lastQuote = optionalArguments.find_last_of( '\"' );
						if ( pos_lastQuote == ( optionalArguments.length() - 1 ) )
						{
							optionalArguments = optionalArguments.substr( 0, optionalArguments.length() - 1 );
						}
						else
						{
							ASSERTF( false, "Expected a trailing quote but didn't find it" );
							break;
						}
					}
				}
			}
		}
		// Create a single command line
		// (Windows doesn't require this, but if the application name is sent to CreateProcess()
		// then it won't show up as the first argument in main(),
		// which causes incorrect behavior to any program expecting standard command arguments)
		std::string commandLine;
		if ( !optionalArguments.empty() )
		{
			std::ostringstream argumentStream;
			argumentStream << path << " " << optionalArguments;
			commandLine = argumentStream.str();
		}
		else
		{
			commandLine = path;
		}
		// Copy it into a non-const buffer
		{
			const size_t argumentLength = commandLine.length() + 1;
			if ( argumentBufferSize >= argumentLength )
			{
				strcpy_s( arguments, argumentBufferSize, commandLine.c_str() );
			}
			else
			{
				ASSERT( false );
				if ( o_errorMessage )
				{
					std::ostringstream errorMessage;
					errorMessage << "The non-const buffer of size " << argumentBufferSize
						<< " isn't big enough to hold the command line of length " << argumentLength;
					*o_errorMessage = errorMessage.str();
				}
				return false;
			}
		}
	}
	
	// Start a new process
	SECURITY_ATTRIBUTES* useDefaultAttributes = NULL;
	const BOOL dontInheritHandles = FALSE;
	const DWORD createDefaultProcess = 0;
	void* useCallingProcessEnvironment = NULL;
	const char* useCallingProcessCurrentDirectory = NULL;
	STARTUPINFO startupInfo = { 0 };
	{
		startupInfo.cb = sizeof( startupInfo );
	}
	PROCESS_INFORMATION processInformation = { 0 };
	if ( CreateProcess( NULL, arguments, useDefaultAttributes, useDefaultAttributes,
		dontInheritHandles, createDefaultProcess, useCallingProcessEnvironment, useCallingProcessCurrentDirectory,
		&startupInfo, &processInformation ) != FALSE )
	{
		// Wait for the process to finish
		if ( WaitForSingleObject( processInformation.hProcess, INFINITE ) != WAIT_FAILED )
		{
			// Get the exit code
			if ( o_exitCode )
			{
				if ( GetExitCodeProcess( processInformation.hProcess, o_exitCode ) == FALSE )
				{
					wereThereErrors = true;
					if ( o_errorMessage )
					{
						const std::string windowsErrorMessage = GetLastSystemError();
						std::ostringstream errorMessage;
						errorMessage << "Windows failed to get the exit code of the process \"" << path <<
							"\": " << windowsErrorMessage;
						*o_errorMessage = errorMessage.str();
					}
				}
			}
		}
		else
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsErrorMessage = GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to wait for the process \"" << path <<
					"\" to finish: " << windowsErrorMessage;
				*o_errorMessage = errorMessage.str();
			}
		}
		// Close the process handles
		if ( CloseHandle( processInformation.hProcess ) == FALSE )
		{
			wereThereErrors = true;
			const std::string windowsErrorMessage = GetLastSystemError();
			ASSERTF( false, "Windows failed to close the handle to the process \"%s\""
				" after executing a command: %s", path.c_str(), windowsErrorMessage.c_str() );
			if ( o_errorMessage )
			{
				std::ostringstream errorMessage;
				errorMessage << "\nWindows failed to close the handle to the process \"" << path <<
					"\": " << windowsErrorMessage;
				*o_errorMessage += errorMessage.str();
			}
		}
		if ( CloseHandle( processInformation.hThread ) == FALSE )
		{
			wereThereErrors = true;
			const std::string windowsErrorMessage = GetLastSystemError();
			ASSERTF( false, "Windows failed to close the handle to the process \"%s\" thread"
				" after executing a command: %s", path.c_str(), windowsErrorMessage.c_str() );
			if ( o_errorMessage )
			{
				std::ostringstream errorMessage;
				errorMessage << "\nWindows failed to close the handle to the process \"" << path <<
					"\" thread: " << windowsErrorMessage;
				*o_errorMessage += errorMessage.str();
			}
		}

		return !wereThereErrors;
	}
	else
	{
		if ( o_errorMessage )
		{
			const std::string windowsErrorMessage = GetLastSystemError();
			std::ostringstream errorMessage;
			errorMessage << "Windows failed to start the process \"" << path << "\": " << windowsErrorMessage;
			*o_errorMessage = errorMessage.str();
		}
		return false;
	}
}

bool Engine::Windows::GetEnvironmentVariable( const char* const i_key, std::string& o_value, std::string* const o_errorMessage )
{
	// Windows requires a character buffer
	// to copy the environment variable into.
	// An arbitrary value is chosen that "should" be "big enough":
	const DWORD maxCharacterCount = 128;
	static char buffer[maxCharacterCount];
	// Ask Windows for the environment variable
	const DWORD characterCount = ::GetEnvironmentVariable( i_key, buffer, maxCharacterCount );
	if ( characterCount > 0 )
	{
		if ( characterCount <= maxCharacterCount )
		{
			o_value = buffer;
			return true;
		}
		else
		{
			// If you're seeing this error you will need to increase maxCharacterCount
			if ( o_errorMessage )
			{
				std::ostringstream errorMessage;
				errorMessage << "The environment variable \"" << i_key << "\" requires " << characterCount <<
					" characters and the provided buffer only has room for " << maxCharacterCount;
				*o_errorMessage = errorMessage.str();
			}
			return false;
		}
	}
	else
	{
		if ( o_errorMessage )
		{
			DWORD errorCode;
			std::string errorString = GetLastSystemError( &errorCode );
			if ( errorCode == ERROR_ENVVAR_NOT_FOUND )
			{
				// If you're seeing this error and the environment variable is spelled correctly
				// it _probably_ means that you are debugging and haven't set up the environment.
				//	* Right click the project name and choose "Properties"
				//	* In the tree view to the left select "Configuration Properties->Debugging"
				//	* In the "Environment" field make sure that you have this environment variable set like:
				//		* someKey=$(someKey)
				std::ostringstream errorMessage;
				errorMessage << "The environment variable \"" << i_key << "\" doesn't exist";
				*o_errorMessage = errorMessage.str();
			}
			else
			{
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to get the environment variable \"" << i_key << "\": " << errorString;
				*o_errorMessage = errorMessage.str();
			}
		}
		return false;
	}
}

std::string Engine::Windows::GetFormattedSystemMessage( const DWORD i_code )
{
	std::string errorMessage;
	{
		const DWORD formattingOptions =
			// Get the error message from Windows
			FORMAT_MESSAGE_FROM_SYSTEM
			// The space for the error message should be allocated by Windows
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			// Any potentially deferred inserts should be ignored
			// (Unfortunately, there are some system messages that have deferred inserts,
			// but there is no way to determine whether a given code has inserts or not
			// and so the only safe way to make a general formatting function like this one
			// is to ignore any)
			| FORMAT_MESSAGE_IGNORE_INSERTS;
		const void* messageIsFromWindows = NULL;
		const DWORD useTheDefaultLanguage = 0;
		char* messageBuffer = NULL;
		const DWORD minimumCharacterCountToAllocate = 1;
		va_list* insertsAreIgnored = NULL;
		const DWORD storedCharacterCount = FormatMessage( formattingOptions, messageIsFromWindows, i_code,
			useTheDefaultLanguage, reinterpret_cast<LPSTR>( &messageBuffer ), minimumCharacterCountToAllocate, insertsAreIgnored );
		if ( storedCharacterCount != 0 )
		{
			errorMessage = messageBuffer;
			// Trailing carriage returns are unnecessary
			errorMessage = errorMessage.substr( 0, errorMessage.find_last_not_of( "\r\n" ) + 1 );
		}
		else
		{
			const DWORD newErrorCode = GetLastError();

			std::ostringstream formattedErrorMessage;
			formattedErrorMessage << "Windows System Code " << i_code;
			if ( newErrorCode == 317 )
			{
				formattedErrorMessage << " (no text description exists)";
			}
			else
			{
				formattedErrorMessage << " (Windows failed to format a text description with error code " << newErrorCode << ")";
			}
			errorMessage = formattedErrorMessage.str();
		}
		// Try to free the memory regardless of whether formatting worked or not,
		// and ignore any error messages
		LocalFree( messageBuffer );
	}
	return errorMessage;
}

std::string Engine::Windows::GetLastSystemError( DWORD* const o_optionalErrorCode )
{
	// Windows stores the error as a code
	const DWORD errorCode = GetLastError();
	if ( o_optionalErrorCode )
	{
		*o_optionalErrorCode = errorCode;
	}
	return GetFormattedSystemMessage( errorCode );
}

bool Engine::Windows::GetLastWriteTime( const char* const i_path, uint64_t& o_lastWriteTime, std::string* const o_errorMessage )
{
	// Get the last time that the file was written to
	ULARGE_INTEGER lastWriteTime;
	{
		WIN32_FIND_DATA fileData;
		{
			HANDLE file = FindFirstFile( i_path, &fileData );
			if ( file != INVALID_HANDLE_VALUE )
			{
				if ( FindClose( file ) == FALSE )
				{
					if ( o_errorMessage )
					{
						*o_errorMessage = GetLastSystemError();
					}
					return false;
				}
			}
			else
			{
				if ( o_errorMessage )
				{
					*o_errorMessage = GetLastSystemError();
				}
				return false;
			}
		}
		FILETIME fileTime = fileData.ftLastWriteTime;
		lastWriteTime.HighPart = fileTime.dwHighDateTime;
		lastWriteTime.LowPart = fileTime.dwLowDateTime;
	}
	o_lastWriteTime = static_cast<uint64_t>( lastWriteTime.QuadPart );
	return true;
}

bool Engine::Windows::InvalidateLastWriteTime( const char* const i_path, std::string* const o_errorMessage )
{
	bool wereThereErrors = false;

	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	{
		const DWORD desiredAccess = FILE_WRITE_ATTRIBUTES;
		const DWORD otherProgramsCanStillReadTheFile = FILE_SHARE_READ;
		SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
		const DWORD onlySucceedIfFileExists = OPEN_EXISTING;
		const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
		const HANDLE dontUseTemplateFile = NULL;
		fileHandle = CreateFile( i_path, desiredAccess, otherProgramsCanStillReadTheFile,
			useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile );
		if ( fileHandle == INVALID_HANDLE_VALUE )
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				*o_errorMessage = GetLastSystemError();
			}
			goto OnExit;
		}
	}
	{
		FILETIME earliestPossibleTime = { 0 };
		{
			SYSTEMTIME systemTime = { 0 };
			{
				systemTime.wYear = 1980;
				systemTime.wMonth = 1;
				systemTime.wDay = 1;
			}
			if ( SystemTimeToFileTime( &systemTime, &earliestPossibleTime ) == FALSE )
			{
				wereThereErrors = true;
				if ( o_errorMessage )
				{
					*o_errorMessage = GetLastSystemError();
				}
				goto OnExit;
			}
		}
		FILETIME* const onlyChangeLastWriteTime = NULL;
		if ( SetFileTime( fileHandle, onlyChangeLastWriteTime, onlyChangeLastWriteTime, &earliestPossibleTime ) == FALSE )
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				*o_errorMessage = GetLastSystemError();
			}
			goto OnExit;
		}
	}

OnExit:

	if ( fileHandle != INVALID_HANDLE_VALUE )
	{
		if ( CloseHandle( fileHandle ) == FALSE )
		{
			wereThereErrors = true;
			const std::string windowsErrorMessage = GetLastSystemError();
			if ( o_errorMessage )
			{
				*o_errorMessage += "\n";
				*o_errorMessage += windowsErrorMessage;
			}
		}
		fileHandle = INVALID_HANDLE_VALUE;
	}

	return !wereThereErrors;
}

bool Engine::Windows::LoadBinaryFile( const char* const i_path, sDataFromFile& o_data, std::string* const o_errorMessage )
{
	bool wereThereErrors = false;

	// Open the file
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	{
		const DWORD desiredAccess = FILE_GENERIC_READ;
		const DWORD otherProgramsCanStillReadTheFile = FILE_SHARE_READ;
		SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
		const DWORD onlySucceedIfFileExists = OPEN_EXISTING;
		const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
		const HANDLE dontUseTemplateFile = NULL;
		fileHandle = CreateFile( i_path, desiredAccess, otherProgramsCanStillReadTheFile,
			useDefaultSecurity, onlySucceedIfFileExists, useDefaultAttributes, dontUseTemplateFile );
		if ( fileHandle == INVALID_HANDLE_VALUE )
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to open the file \"" << i_path << "\" for reading: " << windowsError;
				*o_errorMessage = errorMessage.str();
			}
			goto OnExit;
		}
	}
	// Get the file's size
	{
		LARGE_INTEGER fileSize_integer;
		if ( GetFileSizeEx( fileHandle, &fileSize_integer ) != FALSE )
		{
			ASSERT( fileSize_integer.QuadPart <= SIZE_MAX );
			o_data.size = static_cast<size_t>( fileSize_integer.QuadPart );
		}
		else
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to get the size of the file \"" << i_path << "\": " << windowsError;
				*o_errorMessage = errorMessage.str();
			}
			goto OnExit;
		}
	}
	// Read the file's contents into allocated memory
	o_data.data = malloc( o_data.size );
	if ( o_data.data )
	{
		DWORD bytesReadCount;
		OVERLAPPED* readSynchronously = NULL;
		ASSERT( o_data.size < ( uint64_t( 1u ) << ( sizeof( bytesReadCount ) * 8 ) ) );
		if ( ReadFile( fileHandle, o_data.data, static_cast<DWORD>( o_data.size ),
			&bytesReadCount, readSynchronously ) == FALSE )
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to read the contents of the file \"" << i_path << "\": " << windowsError;
				*o_errorMessage = errorMessage.str();
			}
			goto OnExit;
		}
	}
	else
	{
		wereThereErrors = true;
		if ( o_errorMessage )
		{
			std::ostringstream errorMessage;
			errorMessage << "Failed to allocate " << o_data.size << " bytes to read in the file \"" << i_path << "\"";
			*o_errorMessage = errorMessage.str();
		}
		goto OnExit;
	}

OnExit:

	if ( wereThereErrors )
	{
		if ( o_data.data )
		{
			o_data.Free();
		}
	}
	if ( fileHandle != INVALID_HANDLE_VALUE )
	{
		if ( CloseHandle( fileHandle ) == FALSE )
		{
			if ( !wereThereErrors && o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "\nWindows failed to close the file handle from \"" << i_path << "\": " << windowsError;
				*o_errorMessage += errorMessage.str();
			}
			wereThereErrors = true;
		}
		fileHandle = INVALID_HANDLE_VALUE;
	}

	return !wereThereErrors;
}

bool Engine::Windows::WriteBinaryFile( const char* const i_path, const void* const i_data, const size_t i_size, std::string* const o_errorMessage )
{
	bool wereThereErrors = false;

	// Open the file
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	{
		const DWORD desiredAccess = FILE_GENERIC_WRITE;
		const DWORD noOtherProgramsCanShareAccess = 0;
		SECURITY_ATTRIBUTES* useDefaultSecurity = NULL;
		const DWORD alwaysCreateANewFile = CREATE_ALWAYS;
		const DWORD useDefaultAttributes = FILE_ATTRIBUTE_NORMAL;
		const HANDLE dontUseTemplateFile = NULL;
		fileHandle = CreateFile( i_path, desiredAccess, noOtherProgramsCanShareAccess,
			useDefaultSecurity, alwaysCreateANewFile, useDefaultAttributes, dontUseTemplateFile );
		if ( fileHandle == INVALID_HANDLE_VALUE )
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to open the file \"" << i_path << "\" for writing: " << windowsError;
				*o_errorMessage = errorMessage.str();
			}
			goto OnExit;
		}
	}
	// Write the modified shader source
	{
		OVERLAPPED* writeSynchronously = NULL;
		ASSERT( i_size < ( uint64_t( 1 ) << ( sizeof( DWORD ) * 8 ) ) );
		const DWORD bytesToWriteCount = static_cast<DWORD>( i_size );
		DWORD bytesWrittenCount;
		if ( WriteFile( fileHandle, i_data, bytesToWriteCount, &bytesWrittenCount, writeSynchronously ) != FALSE )
		{
			if ( bytesWrittenCount != bytesToWriteCount )
			{
				wereThereErrors = true;
				if ( o_errorMessage )
				{
					std::ostringstream errorMessage;
					errorMessage << "Windows was supposed to write " << bytesToWriteCount << " bytes to the file \"" << i_path << "\", "
						"but only actually wrote " << bytesWrittenCount;
				}
				goto OnExit;
			}
		}
		else
		{
			wereThereErrors = true;
			if ( o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "Windows failed to write the file \"" << i_path << "\": " << windowsError;
			}
			goto OnExit;
		}
	}

OnExit:

	if ( fileHandle != INVALID_HANDLE_VALUE )
	{
		if ( CloseHandle( fileHandle ) == FALSE )
		{
			if ( !wereThereErrors && o_errorMessage )
			{
				const std::string windowsError = Engine::Windows::GetLastSystemError();
				std::ostringstream errorMessage;
				errorMessage << "\nWindows failed to close the file handle from \"" << i_path << "\": " << windowsError;
				*o_errorMessage += errorMessage.str();
			}
			wereThereErrors = true;
		}
		fileHandle = INVALID_HANDLE_VALUE;
	}

	return !wereThereErrors;
}
