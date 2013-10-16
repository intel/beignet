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

vec3 reflect(vec3 I, vec3 N) {
  return I - 2.0f * dot(N, I) * N;
}

uint pack_fp4(float4 u4) {
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

float jinteresct(vec3 rO, vec3 rD, vec4 c, float *ao)
{
    float mz2,md2,dist,t;
    float res=1000.0f;
    vec4 z,nz;
    int update_ao = 1;
    *ao = 0.0f;
    for(t=0.0f;t<6.0f;t+=dist)
    {
        if (update_ao) *ao += 1.0f;
        vec3 p=rO+t*rD;

        // calc distance
        z=(vec4)(p,(c.y+c.x)*.3f);
        md2=1.0f;
        mz2=dot(z,z);

        for(int i=0;i<9;i++)
        {
             // |dz|^2 -> 4*|dz|^2
             //if (mz2 <= 4.0f)
             {
             md2*=4.0f*mz2;
             // z -> z2 + c
             nz.x=z.x*z.x-dot(z.yzw,z.yzw);
             nz.yzw=2.0f*z.x*z.yzw;
             z=nz+c;
             mz2=dot(z,z);
            }
             if(mz2>4.0f)
                 break;
         }

         dist=0.25f*sqrt(mz2/md2)*log(mz2);
         if(dist<0.0005f)
         {
             res=t;
             break;
         }
         t+= dist;
    }

    return res;
}

#if 1
vec3 calcNormal(vec3 p, vec4 c)
{
    vec4 nz,ndz,dz[4];

    vec4 z=(vec4)(p,(c.y+c.x)*.3f);

    dz[0]=(vec4)(1.0f,0.0f,0.0f,0.0f);
    dz[1]=(vec4)(0.0f,1.0f,0.0f,0.0f);
    dz[2]=(vec4)(0.0f,0.0f,1.0f,0.0f);
  //dz[3]=(vec4)(0.0f,0.0f,0.0f,1.0f);

    for(int i=0;i<9;i++)
    {
        vec4 mz = (vec4)(z.x,-z.y,-z.z,-z.w);
        // derivative
        dz[0]=(vec4)(dot(mz,dz[0]),z.x*dz[0].yzw+dz[0].x*z.yzw);
        dz[1]=(vec4)(dot(mz,dz[1]),z.x*dz[1].yzw+dz[1].x*z.yzw);
        dz[2]=(vec4)(dot(mz,dz[2]),z.x*dz[2].yzw+dz[2].x*z.yzw);
        //dz[3]=(vec4)(dot(mz,dz[3]),z.x*dz[3].yzw+dz[3].x*z.yzw);

        // z = z2 + c
        nz.x=dot(z, mz);
        nz.yzw=2.0f*z.x*z.yzw;
        z=nz+c;

        if(dot(z,z)>4.0f)
            break;
    }

    return normalize((vec3)(dot(z,dz[0]),dot(z,dz[1]),dot(z,dz[2])));
}
#endif

__kernel void compiler_julia(__global uint *dst, float resx, float resy, int w)
{
    vec2 gl_FragCoord = (vec2)(get_global_id(0), get_global_id(1));
    vec2 p=-1.0f+2.0f*gl_FragCoord.xy/(vec2)(resx,resy);
    vec3 color = (vec3)(0.0f);
    vec4 cccc = (vec4)( .7f*cos(.5f*time), .7f*sin(.3f*time), .7f*cos(1.0f*time), 0.0f );
    vec3 edir = normalize((vec3)(p,1.0f));
    vec3 wori = (vec3)(0.0f,0.0f,-2.0f);

    float ao;
    float t = jinteresct(wori,edir,cccc,&ao);
    if(t<100.0f)
    {
#if 1
        vec3 inter = wori + t*edir;
        vec3 nor = calcNormal(inter,cccc);

        float dif = .5f + .5f*dot( nor, (vec3)(0.57703f) );
        ao = max( 1.0f-ao*0.005f, 0.0f);

        color = (vec3)(1.0f,.9f,.5f)*dif*ao +  .5f*(vec3)(.6f,.7f,.8f)*ao;
#else
        color = (vec3)(0.5f,0.0f,0.0f);
#endif
    }
    else
    {
        color = (vec3)(0.5f,0.51f,0.52f)+(vec3)(0.5f,0.47f,0.45f)*p.y;
    }

    vec4 gl_FragColor = (vec4)(color,1.0f);
    OUTPUT;
}

