#version 120

varying vec2 texCoord;
varying vec4 color;

uniform sampler2D texture;
uniform int use_color;

void main()
{
	if(use_color == 1)
	{
		gl_FragColor = color;
	}
	else
	{
		gl_FragColor = texture2D(texture, texCoord);
	}
}
