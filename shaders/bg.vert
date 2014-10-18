attribute vec2 pos;
attribute vec2 texCoords;

varying vec2 uv;

void main() {
	gl_Position = vec4(pos,1.0,1.0);
	uv = texCoords;
}
