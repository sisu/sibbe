attribute vec3 pos;
attribute vec3 color;
attribute vec3 normal;
uniform mat4 trans;
uniform mat3 normalM;
varying vec3 col;
varying vec3 norm;

void main() {
	gl_Position = trans * vec4(pos,1.0);
	col = color;
	norm = normalM * normal;
}
