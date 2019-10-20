/*
	This is an example of a fragment shader
*/

// Constants
//==========

cbuffer constantBuffer : register( b0 )
{
	float g_elapsedSecondCount_total;
}

// Entry Point
//============

void main(

	// Input
	//======

	// The GPU provides us with position

	in float4 i_position : SV_POSITION,

	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : SV_TARGET

	)
{
	// Set the fragment to white
	// (where color is represented by 4 floats representing "RGBA" == "Red/Green/Blue/Alpha").
	// Try experimenting with changing the values of the first three numbers
	// to something in the range [0,1] and observe the results.
	o_color = float4( 1.0, 1.0, 1.0, 1.0 );

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