#ifdef GL_ES
precision highp float;
#endif

uniform vec3 color;

varying vec2 uv;

void main() {
     vec2 t = sqrt(2)/2 - length(vec2(0.5,0.5) - uv);
	gl_FragColor = vec4(color,t);
}
