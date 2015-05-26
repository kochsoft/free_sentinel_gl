#version 120
// Fragment shader for terrain applying the diffuse lighting from terrain.vert.

varying vec2 f_tex_coords;
varying vec4 f_vertex_colors;

uniform sampler2D texture;
uniform float fade;

void main(void)
{
	// texture2D returns a vec4.
	gl_FragColor = texture2D(texture, f_tex_coords) *
	vec4(
		f_vertex_colors.r,
		f_vertex_colors.g,
		f_vertex_colors.b,
		f_vertex_colors.a * fade
	);
}

