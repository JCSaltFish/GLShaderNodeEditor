#version 420

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 fragcolor;

uniform float alpha = 1.0;
uniform vec2 mousePos;
uniform vec4 uCol;
uniform float uTime;
layout(std140, binding = 60) uniform MyBlock
{
	vec4 col;
} testBlock;

void main()
{
	vec3 col = 0.5 + 0.5*cos(uTime+tex_coord.xyx+vec3(0,2,4));
	vec2 coord = tex_coord;
	coord.y = 1. - coord.y;
	vec4 texColor = texture2D(tex, coord);
	//fragcolor = texture2D(tex, coord);
	//fragcolor = vec4(testBlock.col.xyz + uCol.rgb, alpha);
	//fragcolor = vec4(col, 1.0);
	fragcolor = vec4(mousePos, uCol.b + texColor.a, alpha);
}