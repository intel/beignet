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

inline float e(vec3 c)
{
    c=cos((vec3)(cos(c.x+time/6.0f)*c.x-cos(c.y*3.0f+time/5.0f)*c.y,
                 cos(time/4.0f)*c.z/3.0f*c.x-cos(time/7.0f)*c.y,
                 c.x+c.y+c.z+time));
    return dot(c*c,(vec3)(1.0f))-1.0f;
}

__kernel void compiler_nautilus(__global uint *dst, float resx, float resy, int w)
{
  vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
  vec2 c=-1.0f+2.0f*gl_FragCoord.xy/(vec2)(resx,resy);
  vec3 o=(vec3)(c.x,c.y,0.0f),g=(vec3)(c.x,c.y,1.0f)/64.0f,v=(vec3)(0.5f);
  float m = 0.4f;

  for(int r=0;r<100;r++)
  {
    float h=e(o)-m;
    if(h<0.0f)break;
    o+=h*10.0f*g;
    v+=h*0.02f;
  }
  // light (who needs a normal?)
  v+=e(o+0.1f)*(vec3)(0.4f,0.7f,1.0f);

  // ambient occlusion
  float a=0.0f;
  for(int q=0;q<100;q++)
  {
     float l = e(o+0.5f*(vec3)(cos(1.1f*(float)(q)),cos(1.6f*(float)(q)),cos(1.4f*(float)(q))))-m;
     a+=floor(clamp(4.0f*l,0.0f,1.0f));
  }
  v*=a/100.0f;
  vec4 gl_FragColor=(vec4)(v,1.0f);
  OUTPUT;
}

