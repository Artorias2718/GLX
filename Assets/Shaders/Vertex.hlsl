/*
	This is an example of a vertex shader
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

	// The "semantics" (the keywords in all caps after the colon) are arbitrary,
	// but must match the C call to CreateInputLayout()

	// This value comes from one of the sVertex that we filled the vertex buffer with in C code
	in const float2 i_position : POSITION,

	// Output
	//=======

	// An SV_POSITION value must always be output from every vertex shader
	// so that the GPU can figure out which fragments need to be shaded
	out float4 o_position : SV_POSITION
	
    )
{
	// Calculate the position of this vertex on screen
	{
		// When we move to 3D graphics the screen position that the vertex shader outputs
		// will be different than the position that is input to it from C code,
		// but for now the "out" position is set directly from the "in" position:
		o_position = float4( i_position.x, i_position.y, 0.0, 1.0 );
		// Or, equivalently:
		o_position = float4( i_position.xy, 0.0, 1.0 );
		o_position = float4( i_position, 0.0, 1.0 );

		// EAE6320_TODO: Change the color based on time!
		// The value g_elapsedSecondCount_total should change every second, and so by doing something like
		// sin( g_elapsedSecondCount_total ) or cos( g_elapsedSecondCount_total )
		// you can get a value that will oscillate between [-1,1].
		// You should change at least one position element so that the triangle animates.
		// For example, to change X you would do something kind of like:
		//		o_position.x = ? sin( g_elapsedSecondCount_total ) ?
		// You can change .x and .y (but leave .z as 0.0 and .w as 1.0).
		// The screen dimensions are already [1,1], so you may want to do some math
		// on the result of the sinusoid function to keep the triangle mostly on screen.
	}
}