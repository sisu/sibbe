#ifdef GL_ES
precision highp float;
#endif
uniform sampler2D texture;
varying vec2 tc;

void main() {
//	gl_FragColor = vec4(tc,1,1);
	gl_FragColor = texture2D(texture, tc);
//	gl_FragColor = vec4(1,0,0,1);
}
