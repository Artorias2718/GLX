/*
	This file provides more advanced assert functionality than the standard library
*/

#ifndef ASSERTS_H
#define ASSERTS_H

// Header Files
//=============

#include "Configuration.h"

#ifdef ASSERTS_AREENABLED
	#include <intrin.h>
	#include <sstream>
#endif

// Helper Function Declaration
//============================

#ifdef ASSERTS_AREENABLED
	namespace Engine
	{
		namespace Asserts
		{
			// Platform-independent
			bool ShowMessageIfAssertionIsFalseAndReturnWhetherToBreak( const unsigned int i_lineNumber, const char* const i_file,
				bool& io_shouldThisAssertBeIgnoredInTheFuture, const char* const i_message, ... );
			// Platform-specific
			bool ShowMessageAndReturnWhetherToBreak( std::ostringstream& io_message, bool& io_shouldThisAssertBeIgnoredInTheFuture );
		}
	}
#endif

// Interface
//==========

#ifdef ASSERTS_AREENABLED
	#if defined( WINDOWS_API )
		#define ASSERTS_BREAK __debugbreak()
	#else
		#error "No implementation exists for breaking on asserts"
	#endif

	#define ASSERT( i_assertion )	\
	{	\
		static bool shouldThisAssertBeIgnored = false;	\
		if ( !shouldThisAssertBeIgnored && !static_cast<bool>( i_assertion )	\
			&& Engine::Asserts::ShowMessageIfAssertionIsFalseAndReturnWhetherToBreak( __LINE__, __FILE__, shouldThisAssertBeIgnored, "" ) )	\
		{	\
			ASSERTS_BREAK;	\
		}	\
	}
	#define ASSERTF( i_assertion, i_messageToDisplayWhenAssertionIsFalse, ... )	\
	{	\
		static bool shouldThisAssertBeIgnored = false;	\
		if ( !shouldThisAssertBeIgnored && !static_cast<bool>( i_assertion ) \
			&& Engine::Asserts::ShowMessageIfAssertionIsFalseAndReturnWhetherToBreak( __LINE__, __FILE__,	\
				shouldThisAssertBeIgnored, i_messageToDisplayWhenAssertionIsFalse, __VA_ARGS__ ) )	\
		{	\
			ASSERTS_BREAK;	\
		}	\
	}
#else
	// The macros do nothing when asserts aren't enabled
	#define ASSERT( i_assertion )
	#define ASSERTF( i_assertion, i_messageToDisplayWhenAssertionIsFalse, ... )
#endif

#endif	// ASSERTS_H
