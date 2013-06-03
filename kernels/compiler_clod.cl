typedef float2 vec2;
typedef float3 vec3;
typedef float4 vec4;

#define sin native_sin
#define cos native_cos
#define tan native_tan
#define normalize fast_normalize
#define length fast_length
#define mod fmod

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

#define time 1.f

inline float f(vec3 o)
{
    float a=(sin(o.x)+o.y*.25f)*.35f;
    o=(vec3)(cos(a)*o.x-sin(a)*o.y,sin(a)*o.x+cos(a)*o.y,o.z);
    return dot(cos(o)*cos(o),(vec3)(1.f))-1.2f;
}

// XXX front end does not inline this function
inline __attribute((always_inline)) vec3 s(vec3 o,vec3 d)
{
    float t=0.0f;
    float dt = 0.2f;
    float nh = 0.0f;
    float lh = 0.0f;
    for(int i=0;i<50;i++)
    {
        nh = f(o+d*t);
        if(nh>0.0f) { lh=nh; t+=dt; }
    }

    if( nh>0.0f ) return (vec3)(.93f,.94f,.85f);

    t = t - dt*nh/(nh-lh);

    vec3 exyy=(vec3)(0.1f,0.0f,0.0f);
    vec3 eyxy=(vec3)(0.0f,0.1f,0.0f);
    vec3 eyyx=(vec3)(0.0f,0.0f,0.1f);
    vec3 p=o+d*t;
    vec3 n=-normalize((vec3)(f(p+exyy),f(p+eyxy),f(p+eyyx))+(vec3)((sin(p*75.f)))*.01f);

    return (vec3)(mix( ((max(-dot(n,(vec3)(.577f)),0.f) + 0.125f*max(-dot(n,(vec3)(-.707f,-.707f,0.f)),0.f)))*(mod
    (length(p.xy)*20.f,2.f)<1.0f?(vec3)(.71f,.85f,.25f):(vec3)(.79f,.93f,.4f))
                           ,(vec3)(.93f,.94f,.85f), (vec3)(pow(t/9.f,5.f)) ) );
}

#if 0
// XXX vector type in the function arguments not supported yet
__kernel void compiler_clod(__global uint *dst, vec2 resolution, int w)
{
    vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
    //vec2 p = -1.0f + 2.0f * gl_FragCoord.xy / resolution.xy;
    vec2 p;
    p.x = -1.0f + 2.0f * gl_FragCoord.x / resolution.x;
    p.y = -1.0f + 2.0f * gl_FragCoord.y / resolution.y;
    vec4 gl_FragColor=(vec4)(s((vec3)(sin(time*1.5f)*.5f,cos(time)*.5f,time), normalize((vec3)(p.xy,1.0f))),1.0f);
    OUTPUT;
}
#else
__kernel void compiler_clod(__global uint *dst, float resx, float resy, int w)
{
    vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
    //vec2 p = -1.0f + 2.0f * gl_FragCoord.xy / resolution.xy;
    vec2 p;
    p.x = -1.0f + 2.0f * gl_FragCoord.x / resx;
    p.y = -1.0f + 2.0f * gl_FragCoord.y / resy;
    vec4 gl_FragColor=(vec4)(s((vec3)(sin(time*1.5f)*.5f,cos(time)*.5f,time), normalize((vec3)(p.xy,1.0f))),1.0f);
    OUTPUT;
}

#endif

