#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D fft;

varying vec2 uv;
/*
vec4 treeCol(vec2 uv)
{
  vec4 sum = vec4(0,0);
  float t = clamp(texture2D(fft,uv).r,0, 1);

  for(int i = 0; i < 20; ++i) {
    vec3 pos = vec3(uv.x, t, uv.y);

    if(sum.a > 0.99)
      break;

    vec4 col = mapTrees(pos);
    col.xyz = mix(col.xyz, bgcol, 1.0 - exp(-0.0018*t*t));

  }
}
*/
void main() {
  float samplePoint = uv.x*0.4 + 0.2;
 

  float val = texture2D(fft, vec2(samplePoint, 0.25)).r;

  float diff = val/1.1 - uv.y;
  if(diff > 0)
    diff = diff/ ( 0.2025 + (1 - smoothstep(0.1, 0.6, val))*0.5);
  else
    diff = abs(diff)/0.05;

  float w = 1.0 - clamp(diff, 0.0, 1.0);

  vec3 col = vec3( val, 4.0*val*(1.0-val), 1.0-val ) * val;

	
  gl_FragColor = w*vec4(col,1.0);
}
