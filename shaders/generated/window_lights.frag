#version 430
uniform mat4 fg_ModelViewMatrix;
uniform mat4 fg_ModelViewMatrixInverse;
uniform mat4 fg_ProjectionMatrix;
uniform mat4 fg_ModelViewProjectionMatrix;
uniform mat3 fg_NormalMatrix;
layout(location = 0) out vec4 fg_FragColor;
in vec4 fg_Color_vf;
#define gl_Color fg_Color_vf

in float fg_FogFragCoord; // not always used
#define gl_FogFragCoord fg_FogFragCoord
uniform sampler2D tex0;
uniform float min_alpha = 0.0;

in vec2 tc;

float rand(vec2 co) {
   return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}
highp float rand_v2(vec2 co) { // from http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main() {
	vec4 texel = texture(tex0, tc);
	if (texel.a <= min_alpha) discard; // transparent part
	// use integer component of window texture as hash to select random lit vs. dark windows
	float rand_val = rand_v2(vec2(floor(tc.s), floor(tc.y)));
	if (rand_val < 0.67) discard; // 67% of windows are dark
	float alpha     = 1.0 - 2.0*(texel.r - 0.5); // mask off window border (white), keep window pane (gray)
	float rgb_scale = 1.0/min(gl_Color.rgb); // normalize largest color component to 1.0
	fg_FragColor    = vec4(rgb_scale*gl_Color.rgb, alpha*texel.a);
}

