#version 120

varying vec2 texCoord;
varying vec4 color;

uniform sampler2D texture;
uniform int use_color;
uniform int draw_shadow;

void main()
{
	if(draw_shadow == 0)
	{
		gl_FragColor = color;
	}
	else 
	{
		gl_FragColor = vec4(0.1, 0.1, 0.1, 1.0);
	}
}
