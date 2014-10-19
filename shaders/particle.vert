attribute vec3 pos;
attribute vec2 tcoord;

uniform mat4 trans;
uniform vec3 color;

varying vec2 uv;

void main() {
	gl_Position = trans * vec4(pos,1.0);
	uv = tcoord;
}
