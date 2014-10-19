#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D fft;
uniform float avg;

varying vec2 uv;

void main() {
  float samplePoint = uv.x*0.4 + 0.2;
 

  float val = texture2D(fft, vec2(samplePoint, 0.25)).r;

  float diff = val/1.1 - uv.y;
  if(diff > 0.0)
    diff = diff/ ( 0.2025 + (1.0 - smoothstep(0.1, 0.6, val))*0.5);
  else
    diff = abs(diff)/0.05;

  float w = 1.0 - clamp(diff, 0.0, 1.0);

  vec3 col = vec3( val, 4.0*val*(1.0-val), 1.0-val ) * val;

  float w2 = 0.35*smoothstep(0.0, 1.0, 1 - abs(val - uv.y));
  vec3 acol = vec3( avg, 4.0*avg*(1.0-avg), 1.0-avg ) * avg;

  gl_FragColor = w*vec4(col,1.0) + clamp(w2-w+0.1, 0.0, 1.0)*vec4(acol, 1.0);
}
