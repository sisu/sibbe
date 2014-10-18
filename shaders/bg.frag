#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D fft;

varying vec2 uv;

void main() {
  float val = texture(fft, vec2(uv.x, 0.0)).r;

  gl_FragColor = vec4(val, val, val ,1.0);
}
