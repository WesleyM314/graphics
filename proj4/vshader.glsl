#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
varying vec4 color;

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;
uniform int draw_cube;
uniform mat4 cube_transform;
uniform int draw_ball;
uniform mat4 ball_transform;

void main()
{
	color = vColor;
	if(draw_cube == 0)
	{
		if(draw_ball == 1)
		{
			gl_Position = projection * model_view * ball_transform * vPosition;
		}
		else 
		{
			gl_Position = projection * model_view * vPosition;
		}
	}
	else
	{
		gl_Position = projection * model_view * cube_transform * vPosition;
	}
}
