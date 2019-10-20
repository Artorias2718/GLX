// Header Files
//=============

#include "UserSettings.h"

// Static Data Initialization
//===========================

namespace
{
	unsigned int s_resolutionHeight = 512;
	unsigned int s_resolutionWidth = 512;
}

// Interface
//==========

unsigned int Engine::UserSettings::GetResolutionHeight()
{
	return s_resolutionHeight;
}

unsigned int Engine::UserSettings::GetResolutionWidth()
{
	return s_resolutionWidth;
}
