#version 120
/** Vertex shader for the terrain.
 * In this first draft this will use diffuse reflection.
 *
 * http://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Diffuse_Reflection
 */

attribute vec4 v_vertices;
attribute vec3 v_normals;
attribute vec2 v_tex_coords;
attribute vec4 v_vertex_colors;

varying vec2 f_tex_coords;
varying vec4 f_vertex_colors;

uniform vec3 pos_light;
uniform vec4 color_light;
// ambience in [0,1]. How much rest light in shadow? 0.0 for blackness,
// 1.0 for full light. Say something around 0.3.
uniform float ambience;

// The complete transformation of rot->mv->lookAt->perspective.
uniform mat4 A;
// Rotation and Movement alone. Applied to normal vectors. ... m_3x3_inv_transp
uniform mat3 B;

void main(void)
{
	gl_Position = A * v_vertices;
	f_tex_coords = v_tex_coords;

	vec3 dir_normal = normalize(B * v_normals);
	vec3 dir_light = normalize(pos_light.xyz);

	vec3 diffuse_reflection = vec3(color_light.r, color_light.g, color_light.b) *
		max(ambience, dot(dir_normal, dir_light));

	f_vertex_colors = vec4(diffuse_reflection, 1.0) * v_vertex_colors;
}

