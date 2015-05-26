#version 120
// Fragment shader for the sky.
// Lighting free, texture free trivial fragment shader.

varying vec4 f_colors;
uniform float fade;

void main(void)
{
	gl_FragColor = vec4(
		f_colors.r,
		f_colors.g,
		f_colors.b,
		f_colors.a * fade
	);
}

