#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D fft;
uniform float avg;
uniform float precision;

varying vec2 uv;

const float baseSamples = 2000.0;

void main() {
  float p2 = precision * precision;
  float bins = (baseSamples - 10.0)*p2 + 10.0;

  float binSize = 1.0/bins;

  float start = floor(uv.x/binSize);

  float val = 0.0;
  for(int i = 0; i < 10; ++i) {
    float x = (start + float(i)*binSize/10.0)/bins;
    float samplePoint = x*0.4 + 0.2;
    val += texture2D(fft, vec2(samplePoint, 0.25)).r;
  }
  val /= 10.0;

  float diff = val/1.1 - uv.y;
  if(diff > 0.0)
    diff = (p2*diff)/ ( 0.2025 + (1.0 - smoothstep(0.1, 0.6, val))*0.5);
  else
    diff = abs(diff)/(0.05*p2 + 0.00001);

  float w = 1.0 - clamp(diff, 0.0, 1.0);

  vec3 col = vec3( val, 4.0*val*(1.0-val), 1.0-val ) * val;

  float w2 = 0.35*smoothstep(0.0, 1.0, 1.0 - abs(val - uv.y));
  vec3 acol = vec3( avg, 4.0*avg*(1.0-avg), 1.0-avg ) * avg;

  gl_FragColor = w*vec4(col,1.0) + clamp(w2-w+0.1, 0.0, 1.0)*vec4(acol, 1.0);
}
