/*
	This is an example of a vertex shader
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

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// This value comes from one of the sVertex that we filled the vertex buffer with in C code
layout( location = 0 ) in vec2 i_position;

// Output
//=======

// None; the vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an implicit required variable called "gl_Position"

// Entry Point
//============

void main()
{
	// Calculate position
	{
		// When we move to 3D graphics the screen position that the vertex shader outputs
		// will be different than the position that is input to it from C code,
		// but for now the "out" position is set directly from the "in" position:
		gl_Position = vec4( i_position.x, i_position.y, 0.0, 1.0 );
		// Or, equivalently:
		gl_Position = vec4( i_position.xy, 0.0, 1.0 );
		gl_Position = vec4( i_position, 0.0, 1.0 );

		// EAE6320_TODO: Change the color based on time!
		// The value g_elapsedSecondCount_total should change every second, and so by doing something like
		// sin( g_elapsedSecondCount_total ) or cos( g_elapsedSecondCount_total )
		// you can get a value that will oscillate between [-1,1].
		// You should change at least one position element so that the triangle animates.
		// For example, to change X you would do something kind of like:
		//		gl_Position.x = ? sin( g_elapsedSecondCount_total ) ?
		// You can change .x and .y (but leave .z as 0.0 and .w as 1.0).
		// The screen dimensions are already [1,1], so you may want to do some math
		// on the result of the sinusoid function to keep the triangle mostly on screen.
	}
}