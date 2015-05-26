#version 120
// Trivial fragment shader for texture free terrain.

varying vec4 f_vertex_colors;
uniform float fade;

void main(void)
{
	gl_FragColor = vec4(
		f_vertex_colors.r,
		f_vertex_colors.g,
		f_vertex_colors.b,
		f_vertex_colors.a * fade
	);
}

