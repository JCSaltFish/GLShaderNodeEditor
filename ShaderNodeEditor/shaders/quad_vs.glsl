#version 420

out vec2 tex_coord;

const vec4 quad[4] = vec4[]
(
	vec4(-1.0, 1.0, 0.0, 1.0),
	vec4(-1.0, -1.0, 0.0, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(1.0, -1.0, 0.0, 1.0)
);

void main()
{
	gl_Position = quad[gl_VertexID];
	tex_coord = 0.5 * (quad[gl_VertexID].xy + vec2(1.0));
}