typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;
#define sin native_sin
#define cos native_cos
#define tan native_tan
#define normalize fast_normalize
#define length fast_length

inline vec3 reflect(vec3 I, vec3 N) {
  return I - 2.0f * dot(N, I) * N;
}

#define time 1.f

// Object A (tunnel)
inline float oa(vec3 q) {
 return cos(q.x)+cos(q.y*1.5f)+cos(q.z)+cos(q.y*20.f)*.05f;
}

// Object B (ribbon)
inline float ob(vec3 q) {
  return length(max(fabs(q-(vec3)(cos(q.z*1.5f)*.3f,-.5f+cos(q.z)*.2f,.0f))-(vec3)(.125f,.02f,time+3.f),(vec3)(.0f)));
}

// Scene
inline float o(vec3 q) { return min(oa(q),ob(q)); }

// Get Normal XXX Not inline by LLVM
inline __attribute__((always_inline)) vec3 gn(vec3 q) {
 const vec3 fxyy = (vec3)(.01f, 0.f, 0.f);
 const vec3 fyxy = (vec3)(0.f, .01f, 0.f);
 const vec3 fyyx = (vec3)(0.f, 0.f, .01f);
 return normalize((vec3)(o(q+fxyy),
                         o(q+fyxy),
                         o(q+fyyx)));
}

inline uint pack_fp4(float4 u4) {
  uint u;
  u = (((uint) u4.x)) |
      (((uint) u4.y) << 8) |
      (((uint) u4.z) << 16);
  return u;
}

// XXX vector not supported in function argument yet
__kernel void compiler_ribbon(__global uint *dst, float resx, float resy, int w)
{
  vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
  vec2 p = -1.0f + 2.0f * gl_FragCoord.xy / (vec2)(resx, resy);
  p.x *= resx/resy;

  vec4 c = (vec4)(1.0f);
  const vec3 org = (vec3)(sin(time)*.5f,
                          cos(time*.5f)*.25f+.25f,
                          time);
  vec3 dir=normalize((vec3)(p.x*1.6f,p.y,1.0f));
  vec3 q = org, pp;
  float d=.0f;

  // First raymarching
  for(int i=0;i<64;i++) {
    d=o(q);
    q+=d*dir;
  }
  pp=q;
  const float f = length(q-org)*0.02f;

  // Second raymarching (reflection)
  dir=reflect(dir,gn(q));
  q+=dir;
  for(int i=0;i<64;i++) {
    d=o(q);
    q+=d*dir;
  }
  c = max(dot(gn(q), (vec3)(0.1f,0.1f,0.0f)), 0.0f)
    + (vec4)(0.3f, cos(time*.5f)*.5f+.5f, sin(time*.5f)*.5f+.5f, 1.f) * min(length(q-org)*.04f,1.f);

  // Ribbon Color
  if(oa(pp)>ob(pp))
    c = mix(c, (vec4)(cos(time*.3f)*0.5f + 0.5f,cos(time*.2f)*.5f+.5f,sin(time*.3f)*.5f+.5f,1.f),.3f);

  // Final Color
  const vec4 color = ((c+(vec4)(f))+(1.f-min(pp.y+1.9f,1.f))*(vec4)(1.f,.8f,.7f,1.f))*min(time*.5f,1.f);
  const vec4 final = 255.f * max(min(color, (vec4)(1.f)), (vec4)(0.f));
  dst[get_global_id(0) + get_global_id(1) * w] = pack_fp4(final);
}
