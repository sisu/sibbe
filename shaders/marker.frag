#ifdef GL_ES
precision highp float;
#endif
varying vec3 col;
varying vec3 norm;
void main() {
	float s3=0.57735026919;
	vec3 light = vec3(s3,s3,-s3);
	float l = 0.2 + 0.8 * max(0.0, dot(normalize(norm), light));
	float l2 = dot(col,col)>0.2 ? l*l : 0.0;
	gl_FragColor = vec4(l*col + l2*vec3(1,1,1),1.0);
//	gl_FragColor = vec4(l,l,l,1.0);
}
