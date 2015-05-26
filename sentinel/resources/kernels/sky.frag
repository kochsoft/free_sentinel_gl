#version 120
// Fragment shader for the sky.
// Since lighting plays no role this shader simply applies the
// light color passed through from the vertex shader onto the
// texture color. It applies the vertex color in the same way.

varying vec2 f_tex_coords;
varying vec4 f_colors;

uniform sampler2D texture;
uniform float fade;

void main(void)
{
	gl_FragColor = texture2D(texture, f_tex_coords) *
	vec4(
		f_colors.r,
		f_colors.g,
		f_colors.b,
		f_colors.a * fade
	);
}

