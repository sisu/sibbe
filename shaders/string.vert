attribute vec3 pos;
attribute vec3 color;
attribute vec3 normal;
attribute float spiral;

uniform mat4 trans;
uniform mat3 normalM;

varying vec3 col;
varying vec3 norm;
varying float sp;

void main() {
	gl_Position = trans * vec4(pos,1.0);
	col = color;
	norm = normalM * normal;
	sp = spiral;
}
