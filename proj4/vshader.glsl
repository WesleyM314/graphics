#version 120

attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vNormal;
varying vec4 color;

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;

uniform int draw_cube;
uniform mat4 cube_transform;

uniform int draw_ball;
uniform mat4 ball_transform;

uniform vec4 ambient_product, diffuse_product, specular_product;
uniform float shininess;
uniform float attenuation_constant, attenuation_linear, attenuation_quadratic;
vec4 ambient, diffuse, specular;

uniform int draw_shadow;
uniform vec4 light_position;
uniform float shadow_plane_y;
uniform int draw_plane;

void main()
{
	color = vColor;
	if(draw_cube == 0 && draw_plane == 0)
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
			
			// Calculate color based on lighting
			// ambient = ambient_product;
			ambient = vColor * 0.6;

			// Get relative normal
			vec4 N = normalize(model_view * cube_transform * vNormal);
			
			// Light source position fixed to object frame
			vec4 L_temp = model_view * (light_position - (cube_transform * vPosition));
			vec4 L = normalize(L_temp);
			// diffuse = max(dot(L, N), 0.0) * diffuse_product;
			diffuse = max(dot(L, N), 0.0) * vColor;

			vec4 eye_position = vec4(0.0, 0.0, 0.0, 1.0);
			vec4 V = normalize(eye_position - (model_view * cube_transform * vPosition));
			vec4 H = normalize(L + V);
			// specular = pow(max(dot(N,H), 0.0), shininess) * specular_product;
			specular = pow(max(dot(N,H), 0.0), shininess) * vec4(1,1,1,1);

			float distance = length(L_temp);
			// For now, set attenuation to 1
			float attenuation = 0.8;

			color = ambient + (attenuation * (diffuse + specular));

			// If plane, ignore lighting
			if(draw_plane == 1)
			{
				color = vColor;
			}
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
