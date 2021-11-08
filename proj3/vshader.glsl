#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec2 vTexCoord;
varying vec2 texCoord;
varying vec4 color;

uniform mat4 ctm;

void main()
{
	texCoord = vTexCoord;
	color = vColor;
	gl_Position = ctm * vPosition;
}
