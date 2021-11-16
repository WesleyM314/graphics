#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec2 vTexCoord;
varying vec2 texCoord;
varying vec4 color;

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;
uniform int draw_arrow;
uniform mat4 arrow_tr;

void main()
{
	texCoord = vTexCoord;
	color = vColor;
	if(draw_arrow == 0)
	{
		gl_Position = projection * model_view * vPosition;
	}
	else
	{
		// Translate points by arrow_tr mat
		gl_Position = projection * model_view * arrow_tr * vPosition;
	}
}
