#version 330 core 

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 inpColor;

vec3 sphereToCart(vec3 v0) {
	vec3 v1;
	v1.x = v0.x * cos(v0.y) * sin(v0.z);
	v1.y = v0.x * sin(v0.y) * sin(v0.z);
	v1.z = v0.x * cos(v0.z);
	return v1;
}	

void main() {
	gl_Position = vec4(normalize(position), 1.0f);
	inpColor = 0.5 * color;
}
