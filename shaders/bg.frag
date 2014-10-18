#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D fft;

varying vec2 uv;

void main() {
  float val = texture2D(fft, vec2(uv.x*0.4+.2, 0.0)).r/1.2;

  float diff = 1.0 - clamp(abs(val - uv.y)/0.03, 0.0, 1.0);

  gl_FragColor = vec4(diff, diff, diff ,1.0);
}
