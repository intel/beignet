typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;

#define sin native_sin
#define cos native_cos
#define tan native_tan
#define normalize fast_normalize
#define length fast_length
#define mod fmod
#define time 10.f

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

__kernel void compiler_chocolux(__global uint *dst, float resx, float resy, int w)
{
  vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
  vec3 s[4];
  s[0]=(vec3)(0);
  s[3]=(vec3)(sin(time),cos(time),0);
  s[1]=s[3].zxy;
  s[2]=s[3].zzx;

  float t,b,c,h=0.0f;
  vec3 m,n;
  vec3 p=(vec3)(.2f);
  vec3 d=normalize(.001f*(vec3)(gl_FragCoord,.0f)-p);

  for(int i=0;i<4;i++)
  {
    t=2.0f;
    for(int i=0;i<4;i++)
    {
      b=dot(d,n=s[i]-p);
      c=b*b+.2f-dot(n,n);
      if(b-c<t)
      if(c>0.0f)
      {
        m=s[i];t=b-c;
      }
    }
    p+=t*d;
    d=reflect(d,n=normalize(p-m));
    h+=pow(n.x*n.x,44.f)+n.x*n.x*.2f;
  }
  vec4 gl_FragColor=(vec4)(h,h*h,h*h*h*h,1.f);
  OUTPUT;
}

