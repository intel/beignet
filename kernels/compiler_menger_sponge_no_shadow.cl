// See http://www.iquilezles.org/articles/menger/menger.htm for the 
// full explanation of how this was done

typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;

#define sin native_sin
#define cos native_cos
#define tan native_tan
#define normalize fast_normalize
#define length fast_length
#define mod fmod
#define time 1.f

// fmod is not like glsl mod!
inline __attribute__((always_inline, overloadable))
float glsl_mod(float x,float y) { return mad( -y, floor(x/y), x); }
inline __attribute__((always_inline, overloadable))
float2 glsl_mod(float2 a,float2 b) { return (float2)(glsl_mod(a.x,b.x), glsl_mod(a.y,b.y)); }
inline __attribute__((always_inline, overloadable))
float3 glsl_mod(float3 a,float3 b) { return (float3)(glsl_mod(a.x,b.x), glsl_mod(a.y,b.y), glsl_mod(a.z,b.z)); }

inline vec3 reflect(vec3 I, vec3 N) {
  return I - 2.0f * dot(N, I) * N;
}

inline uint pack_fp4(float4 u4) {
  uint u;
  u = (((uint) u4.x)) |
      (((uint) u4.y) << 8) |
      (((uint) u4.z) << 16);
  return u;
}

#define OUTPUT do {\
  const vec4 final = 255.f * max(min(gl_FragColor, (vec4)(1.f)), (vec4)(0.f)); \
  dst[get_global_id(0) + get_global_id(1) * w] = pack_fp4(final); \
} while (0)

inline __attribute__((always_inline))
float maxcomp(vec3 p) { return max(p.x,max(p.y,p.z));}

inline __attribute__((always_inline))
float sdBox(vec3 p, vec3 b)
{
  vec3  di = fabs(p) - b;
  float mc = maxcomp(di);
  return min(mc,length(max(di,0.0f)));
}

inline __attribute__((always_inline))
vec4 map(vec3 p)
{
   float d = sdBox(p,(vec3)(1.0f));
   float4 res = (vec4)(d,1.f,0.f,0.f);

   float s = 1.0f;
   for( int m=0; m<3; m++ ) 
   {
      vec3 a = glsl_mod(p*s, 2.0f)-1.0f;
      s *= 3.0f;
      float rx = fabs(1.0f - 3.0f*fabs(a.x));
      float ry = fabs(1.0f - 3.0f*fabs(a.y));
      float rz = fabs(1.0f - 3.0f*fabs(a.z));

      float da = max(rx,ry);
      float db = max(ry,rz);
      float dc = max(rz,rx);
      float c = (min(da,min(db,dc))-1.0f)/s;
      if (c > d)
      {
          d = c;
          res = (vec4)(d, 0.2f*da*db*dc, (1.0f+(float)(m))/4.0f, 0.0f);
      }
   }
   return (vec4)(res.x,res.y,res.z,0.f);
}

// GLSL ES doesn't seem to like loops with conditional break/return...
inline __attribute__((always_inline))
vec4 intersect( vec3 ro, vec3 rd )
{
    float t = 0.0f;
    for(int i=0;i<64;i++)
    {
        vec4 h = map(ro + rd*t);
        if( h.x<0.002f )
            return (vec4)(t,h.yzw);
        t += h.x;
    }
    return (vec4)(-1.0f);
}

__kernel void compiler_menger_sponge_no_shadow(__global uint *dst, float resx, float resy, int w)
{
    vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
    vec2 p=-1.0f+2.0f*gl_FragCoord.xy/(vec2)(resx,resy);

    // light
    vec3 light = normalize((vec3)(1.0f,0.8f,-0.6f));

    float ctime = time;
    // camera
    vec3 ro = 1.1f*(vec3)(2.5f*cos(0.5f*ctime),1.5f*cos(ctime*.23f),2.5f*sin(0.5f*ctime));
    vec3 ww = normalize((vec3)(0.0f) - ro);
    vec3 uu = normalize(cross( (vec3)(0.0f,1.0f,0.0f), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5f*ww );
    vec3 col = (vec3)(0.0f);
    vec4 tmat = intersect(ro,rd);

    if( tmat.x>0.0f )
        col = (vec3)(
            0.6f+0.4f*cos(5.0f+6.2831f*tmat.z),
            0.6f+0.4f*cos(5.4f+6.2831f*tmat.z),
            0.6f+0.4f*cos(5.7f+6.2831f*tmat.z) );

  vec4 gl_FragColor = (vec4)(col,1.0f);
  OUTPUT;
}


