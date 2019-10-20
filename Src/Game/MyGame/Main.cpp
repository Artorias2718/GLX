/*
	WinMain() is the entry point of a Windows program
	(instead of the familiar main() in a console program)
*/

// Header Files
//=============

#include "cGame.h"
#include <stdio.h>

// Entry Point
//============

// To swap between Main Functions, go to the Startup Project -> Properties -> Linker -> System
// For a console application, select Console for the Subsystem mode

int main(int argc, char** argv)
{
	printf("Hello, World!");
	return 0;
}

// For a windows application, select Windows for the Subsystem mode

//int WINAPI WinMain(HINSTANCE i_thisInstanceOfTheApplication, HINSTANCE, char* i_commandLineArguments, int i_initialWindowDisplayState)
//{
//	return Engine::Application::Run<Engine::cGame>(i_thisInstanceOfTheApplication, i_commandLineArguments, i_initialWindowDisplayState);
//}
