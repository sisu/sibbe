#ifdef GL_ES
precision highp float;
#endif

varying vec3 col;
varying vec3 norm;
varying float sp;

void main() {
	float s3=0.57735026919;
	vec3 light = vec3(s3,s3,-s3);

	float f = fract(sp);
	float p = clamp(2.0*(abs(0.5 - fract(sp)) + 0.3), 0.0, 1.0);


	float l =  0.1 + p * 0.9 * max(0.0, dot(normalize(norm), light));
	gl_FragColor = vec4(l*col,1.0);
}
