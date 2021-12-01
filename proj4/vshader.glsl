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

uniform int draw_shadow;
uniform vec4 light_position;
uniform float shadow_plane_y;

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
		if(draw_shadow == 0)
		{
			gl_Position = projection * model_view * cube_transform * vPosition;
		}
		else 
		{
			// First apply cube_transform
			vec4 p = cube_transform * vPosition;
			// Do calculations to flatten for shadow
			float x, z;

			x = light_position.x - (light_position.y - shadow_plane_y) * ((light_position.x - p.x) / (light_position.y - p.y));

			z = light_position.z - (light_position.y - shadow_plane_y) * ((light_position.z - p.z) / (light_position.y - p.y));

			gl_Position = projection * model_view  * vec4(x, shadow_plane_y, z, 1.0);
		}
	}
}
