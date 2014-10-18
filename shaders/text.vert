attribute vec3 pos;
attribute vec2 tcoord;
uniform mat4 trans;

varying vec2 tc;

void main() {
	gl_Position = trans * vec4(pos,1.0);
	tc = tcoord;
}
