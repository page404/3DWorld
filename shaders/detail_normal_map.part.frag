uniform vec2 detail_normal_tex_scale = vec2(8.0);
uniform sampler2D detail_normal_tex;

in vec2 tc;

vec3 get_bump_map_normal_dnm(in float bump_scale, in vec3 eye_pos) {
	vec3 nmap = texture(detail_normal_tex, detail_normal_tex_scale*tc).rgb; // scaled detail texture
#ifdef BLEND_DIST_DETAIL_NMAP // mix in a lower frequency normal map sample to break up tiling artifacts
	vec3 nmap2= texture(detail_normal_tex, detail_normal_tex_scale*tc/6.0).rgb;
	nmap = mix(nmap, nmap2, clamp((length(eye_pos) - 0.8), 0.0, 1.0));
#endif
	return normalize(mix(vec3(0,0,1), (2.0*nmap - 1.0), bump_scale));
}
vec3 get_bump_map_normal() {
	return get_bump_map_normal_dnm(1.0, vec3(0,0,1)); // FIXME: eye_pos is not valid
}
mat3 get_tbn(in float bscale, in vec3 n) {
	vec3 tan = normalize(cross(fg_ModelViewMatrix[0].xyz, n));
	return transpose(mat3(bscale*cross(n, tan), -tan, normalize(n))); // world space {X, -Y, Z} for normal in +Z
}
vec3 apply_bump_map(inout vec3 light_dir, inout vec3 eye_pos, in vec3 n, in float bump_scale) {
	mat3 TBN  = get_tbn(1.0, n);
	light_dir = normalize(TBN * light_dir);
	eye_pos   = TBN * eye_pos;
	return get_bump_map_normal_dnm(bump_scale, eye_pos);
}
vec3 apply_bump_map_for_tbn(inout vec3 light_dir, inout vec3 eye_pos, in mat3 TBN) {
	light_dir = normalize(TBN * light_dir);
	eye_pos   = TBN * eye_pos;
	return get_bump_map_normal(); // in tangent space
}
