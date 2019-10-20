/*
	This is an example of a fragment shader
*/

// The version of GLSL to use must come first
#version 420

// Constants
//==========

layout( std140, binding = 0 ) uniform constantBuffer
{
	float g_elapsedSecondCount_total;
};

// Input
//======

// None; the only data that the vertex shader output was gl_Position,
// and that is used by the GPU

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Entry Point
//============

void main()
{
	// For now set the fragment to white
	// (where color is represented by 4 floats representing "RGBA" == "Red/Green/Blue/Alpha").
	// If you are curious you should experiment with changing the values of the first three numbers
	// to something in the range [0,1] and observing the results
	// (although when you submit your Assignment 01 the color output must be white).
    o_color = vec4( 1.0, 1.0, 1.0, 1.0 );

	// EAE6320_TODO: Change the color based on time!
	// The value g_elapsedSecondCount_total should change every second, and so by doing something like
	// sin( g_elapsedSecondCount_total ) or cos( g_elapsedSecondCount_total )
	// you can get a value that will oscillate between [-1,1].
	// You should change at least one "channel" so that the color animates.
	// For example, to change red ("r") you would do something kind of like:
	//		o_color.r = ? sin( g_elapsedSecondCount_total ) ?
	// You can change .r, .g, and .b (but leave .a as 1.0).
	// Remember that your final values should be [0,1], so you will have to do some math.
}