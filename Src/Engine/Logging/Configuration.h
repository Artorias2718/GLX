/*
	This file provides configurable settings
	that can be used to control logging behavior
*/

#ifndef LOGGING_CONFIGURATION_H
#define LOGGING_CONFIGURATION_H

// By default the log uses a relative path which will be generated
// in the same directory as the game's executable
// (which is nice because it is easy for a user to find)
#define LOGGING_DEFAULTPATH "Engine.log"

// Flushing the logging buffer to disk is expensive,
// but it can be done after every message is output during development
// if an application is crashing so that no messages are lost
#ifdef _DEBUG
	#define LOGGING_FLUSHBUFFERAFTEREVERYMESSAGE
#endif

#endif	// LOGGING_CONFIGURATION_H
