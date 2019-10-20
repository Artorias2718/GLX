// Header Files
//=============

#include "Logging.h"

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "../Asserts/Asserts.h"

// Helper Function Declarations
//=============================

namespace
{
	bool OutputMessage( const char* const i_message, va_list io_insertions );
}

// Helper Class Declaration
//=========================

namespace
{
	class cLogging
	{
		// Interface
		//----------

	public:

		// Logging
		bool OutputMessage( const std::string& i_message );
		void FlushLog();
		// Initialization / Clean Up
		bool InitializeIfNecesary( const char* const i_pathToLogTo = NULL );
		static void CleanUp();
		~cLogging();

		// Data
		//-----

	private:

		std::ofstream m_outputStream;
	};
}

// Static Data Initialization
//===========================

namespace
{
	std::string s_path_log;
	cLogging s_logger;
	bool s_hasTheLoggerBeenDestroyed = false;
	bool s_hasTheLogFileAlreadyBeenWrittenTo = false;
	bool s_cleanUpResult = false;
}

// Interface
//==========

// Output
//-------

bool Engine::Logging::OutputMessage( const char* const i_message, ... )
{
	bool result;
	{
		va_list insertions;
		va_start( insertions, i_message );
		result = ::OutputMessage( i_message, insertions );
		va_end( insertions );
	}
#ifdef LOGGING_FLUSHBUFFERAFTEREVERYMESSAGE
	s_logger.FlushLog();
#endif
	return result;
}

bool Engine::Logging::OutputError( const char* const i_errorMessage, ... )
{
	bool result;
	{
		va_list insertions;
		va_start( insertions, i_errorMessage );
		result = ::OutputMessage( i_errorMessage, insertions );
		va_end( insertions );
	}
	s_logger.FlushLog();
	return result;
}

// Initialization / Clean Up
//--------------------------

bool Engine::Logging::Initialize( const char* const i_pathToLogTo )
{
	return s_logger.InitializeIfNecesary( i_pathToLogTo );
}

bool Engine::Logging::CleanUp()
{
	cLogging::CleanUp();
	return s_cleanUpResult;
}

// Helper Function Definitions
//============================

namespace
{
	bool OutputMessage( const char* const i_message, va_list io_insertions )
	{
		const size_t bufferSize = 512;
		char buffer[bufferSize];
		const int formattingResult = vsnprintf( buffer, bufferSize, i_message, io_insertions );
		if ( formattingResult >= 0 )
		{
			if ( formattingResult < bufferSize )
			{
				return s_logger.OutputMessage( buffer );
			}
			else
			{
				ASSERTF( false, "The internal logging buffer of size %u was not big enough to hold the formatted message of length %i",
					bufferSize, formattingResult + 1 );
				std::ostringstream errorMessage;
				errorMessage << "FORMATTING ERROR! (The internal logging buffer of size " << bufferSize
					<< " was not big enough to hold the formatted message of length " << ( formattingResult + 1 ) << ".)"
					" Cut-off message is:\n\t" << buffer;
				s_logger.OutputMessage( errorMessage.str().c_str() );
				// Return false regardless of whether the unformatted message was output
				return false;
			}
		}
		else
		{
			ASSERTF( false, "An encoding error occurred while logging the message \"%s\"", i_message );
			std::ostringstream errorMessage;
			errorMessage << "ENCODING ERROR! Unformatted message was:\n\t" << i_message;
			s_logger.OutputMessage( errorMessage.str().c_str() );
			// Return false regardless of whether the unformatted message was output
			return false;
		}
	}
}

// Helper Class Definition
//========================

namespace
{
	// Interface
	//----------

	// Logging

	bool cLogging::OutputMessage( const std::string& i_message )
	{
		if ( InitializeIfNecesary() )
		{
			// Write the message to the file
			m_outputStream << i_message << "\n";

			return true;
		}
		else
		{
			return false;
		}
	}

	inline void cLogging::FlushLog()
	{
		m_outputStream.flush();
	}

	// Initialization / Clean UP

	bool cLogging::InitializeIfNecesary( const char* const i_pathToLogTo )
	{
		// If there is an error when the application is exiting
		// the logger may have already been destroyed
		if ( !s_hasTheLoggerBeenDestroyed )
		{
			if ( m_outputStream.is_open() )
			{
				if ( !i_pathToLogTo )
				{
					return true;
				}
				else
				{
					const bool arePathsEqual = s_path_log.compare( i_pathToLogTo ) == 0;
					ASSERTF( arePathsEqual, "The log file \"%s\" is already open,"
						" but an attempt is being made to initialize logging with the different path \"%s\"",
						s_path_log.c_str(), i_pathToLogTo );
					Engine::Logging::OutputError( "Error! An attempt was made to initialize logging with the path \"%s\""
						" after the current log file had already been opened with this path", i_pathToLogTo );
					return arePathsEqual;
				}
			}
			else
			{
				if ( !s_hasTheLogFileAlreadyBeenWrittenTo )
				{
					if ( i_pathToLogTo )
					{
						if ( i_pathToLogTo[0] == '\0' )
						{
							ASSERT( "An attempt is being made to initialize logging with an empty path" );
							return false;
						}
						if ( !s_path_log.empty() )
						{
							const bool arePathsEqual = s_path_log.compare( i_pathToLogTo ) == 0;
							ASSERTF( arePathsEqual, "An attempt to initialize logging with a log file path of \"%s\" has already been made,"
								" but another attempt to initialize it is being made with the different path \"%s\"",
								s_path_log.c_str(), i_pathToLogTo );
							// Although probably a mistake this doesn't prevent the application from running
						}
						s_path_log = i_pathToLogTo;
					}
					else
					{
						s_path_log = LOGGING_DEFAULTPATH;
					}
					ASSERTF( !s_path_log.empty(), "The path to log to is empty" );
					m_outputStream.open( s_path_log.c_str() );
					if ( m_outputStream.is_open() )
					{
						s_hasTheLogFileAlreadyBeenWrittenTo = true;
						Engine::Logging::OutputMessage( "Opened log file \"%s\"", s_path_log.c_str() );
						FlushLog();
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					if ( i_pathToLogTo )
					{
						const bool arePathsEqual = s_path_log.compare( i_pathToLogTo ) == 0;
						ASSERTF( arePathsEqual, "The log file \"%s\" has already been written to,"
							" but an attempt is being made to initialize logging with the different path \"%s\"",
							s_path_log.c_str(), i_pathToLogTo );
						if ( !arePathsEqual )
						{
							return false;
						}
					}
					m_outputStream.open( s_path_log.c_str(), std::ofstream::app );
					const bool result = m_outputStream.is_open();
					ASSERT( result );
					if ( result )
					{
						Engine::Logging::OutputMessage( "Re-opened log file" );
						FlushLog();
					}
					return result;
				}
			}
		}
		else
		{
			// If the logger has already been destroyed it needs to be re-constructed in the same memory location
			{
				new ( this ) cLogging;
				// Register the CleanUp() function so that it still gets called when the application exits
				// (the destructor won't be called automatically when using placement new)
				{
					int result = atexit( CleanUp );
					if ( result == 0 )
					{
						s_hasTheLoggerBeenDestroyed = false;
					}
					else
					{
						ASSERTF( false, "Calling atexit() to register logging clean up on a revived logger failed with a return value of %i", result );
						CleanUp();
						return false;
					}
				}
			}
			// Re-open the file
			m_outputStream.open( s_path_log.c_str(), std::ofstream::app );
			const bool result = m_outputStream.is_open();
			ASSERT( result );
			if ( result )
			{
				Engine::Logging::OutputMessage( "Re-opened log file after it had been destroyed" );
				FlushLog();
			}
			return result;
		}
	}

	void cLogging::CleanUp()
	{
		if ( s_logger.m_outputStream.is_open() )
		{
			{
				Engine::Logging::OutputMessage( "Closing log file" );
				s_logger.FlushLog();
			}
			s_logger.m_outputStream.close();
			s_cleanUpResult = !s_logger.m_outputStream.is_open();
			ASSERTF( s_cleanUpResult, "Log file wasn't closed" );
			if ( !s_cleanUpResult )
			{
				Engine::Logging::OutputError( "Error: Log file did not close" );
			}
			return;
		}

		s_cleanUpResult = true;
	}

	cLogging::~cLogging()
	{
		s_hasTheLoggerBeenDestroyed = true;
		CleanUp();
	}
}
