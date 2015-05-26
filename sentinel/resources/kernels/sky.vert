#version 120
/** Vertex shader for the sky.
 * The sky might be a simple half-dome with a texture.
 * Here is a nice tutorial how to generate one in Blender:
 *   https://www.youtube.com/watch?v=5HX0bJ14Dpg
 * The tutorial even contains a hint as to how to have the
 * normal vectors pointing to the inside (which will be
 * interesting if I ever use a kind of culling mode where
 * only the top-side of surfaces is to be displayed).
 *
 * This shader pair (sky.vert and sky.frag) displays the
 * sky in the given ambient light color but will not
 * respect the angle of incoming light.
 *
 * http://stackoverflow.com/questions/16620013/should-i-calculate-matrices-on-the-gpu-or-on-the-cpu
 * states a nice rule of thumb concerning the calculate
 * matrices on CPU or GPU question: If it can be passed as a
 * uniform it should be done on the CPU.
 */

attribute vec4 v_vertices;
attribute vec2 v_tex_coords;
attribute vec4 v_colors;

varying vec2 f_tex_coords;
varying vec4 f_colors;

uniform mat4 A;
// Transparent light?! Think about the effect: It makes the world translucent!
uniform vec4 color_light;

void main(void)
{
	gl_Position = A*v_vertices;
	f_tex_coords = v_tex_coords;
	f_colors = v_colors * color_light;
}

