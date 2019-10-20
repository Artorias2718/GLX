/*
	This file manages time-related functionality
*/

#ifndef TIME_H
#define TIME_H

// Interface
//==========

namespace Engine
{
	namespace Time
	{
		// Time
		//-----

		float GetElapsedSecondCount_total();
		float GetElapsedSecondCount_duringPreviousFrame();

		void OnNewFrame();

		// Initialization / Clean Up
		//--------------------------

		bool Initialize();
		bool CleanUp();
	}
}

#endif	// TIME_H
