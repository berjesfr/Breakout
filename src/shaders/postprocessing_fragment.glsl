#version 450 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D scene;
uniform vec2 offsets[9];
uniform int edge_kernel[9];
uniform float blur_kernel[9];

uniform bool chaos;
uniform bool shake;
uniform bool confuse;

void main() {

	color = vec4(0.0);
	vec3 samp[9];
	if (chaos || shake)
		for(int i = 0; i < 9; i++)
			samp[i] = vec3(texture(scene, TexCoords.st + offsets[i]));

	if (chaos) {
		for(int i = 0; i < 9; i++) {
			color += vec4(samp[i] * edge_kernel[i], 0.0);
		}
		color.a = 1.0;
	} else if (confuse) {
		color = vec4(1.0 - texture(scene, TexCoords).rgb, 1.0);
	} else if (shake) {
		for (int i = 0; i < 9; i++)
			color += vec4(samp[i] * blur_kernel[i], 0.0);
		color.a = 1.0;
	} else {
		color = texture(scene, TexCoords);
	}
}