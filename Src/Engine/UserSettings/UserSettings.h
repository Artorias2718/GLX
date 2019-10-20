/*
	This file manages user-configurable settings
	(ones that the user can set in an INI file)
*/

#ifndef USERSETTINGS_H
#define USERSETTINGS_H

// Interface
//==========

namespace Engine
{
	namespace UserSettings
	{
		unsigned int GetResolutionHeight();
		unsigned int GetResolutionWidth();
	}
}

#endif	// USERSETTINGS_H
