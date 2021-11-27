#version 120

varying vec2 texCoord;
varying vec4 color;

uniform sampler2D texture;
uniform int use_color;

void main()
{
	gl_FragColor = color;
}
