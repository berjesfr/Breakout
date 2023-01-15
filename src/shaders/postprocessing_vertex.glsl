#version 450 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform bool chaos;
uniform bool shake;
uniform bool confuse;
uniform float time;

void main() {

	gl_Position = vec4(vertex.xy, 0.0, 1.0);
	vec2 tex = vertex.zw;
	if (chaos) {
		float strength = 0.3;
		vec2 pos = vec2(tex.x + sin(time) * strength, tex.y + cos(time) * strength);
		TexCoords = pos;
	} else if (confuse) {
		TexCoords = vec2(1.0 - tex.x, 1.0 - tex.y);
	} else {
		TexCoords = tex;
	}
	if (shake) {
		float strength = 0.01;
		gl_Position.x += cos(time * 10) * strength;
		gl_Position.y += cos(time * 15) * strength;
	}


}