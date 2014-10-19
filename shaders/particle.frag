#ifdef GL_ES
precision highp float;
#endif

uniform vec3 color;
uniform float age;

varying vec2 uv;

void main() {
	float t = max(0.0, .5 - length(vec2(0.5,0.5) - uv));
	gl_FragColor = vec4(color,t*age);
}
