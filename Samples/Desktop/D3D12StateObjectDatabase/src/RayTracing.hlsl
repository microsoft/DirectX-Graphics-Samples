RaytracingAccelerationStructure MyAccelerationStructure : register(t0);
RWTexture2D<float4> RT : register(u0);
RWByteAddressBuffer scratch : register(u1);
RWByteAddressBuffer scratch2 : register(u2);
struct MyPayload
{
    float4 color;
};

float bar2(float b)
{
    return 2*b;
}

float4 bar(float b)
{
    return float4(b, b, b, b);
}


float4 foo(int a, float b)
{
    return float4(b, b, (float)a, bar(b).x + scratch2.Load(0) );
}

void touchScratch()
{
    scratch.Store(4,scratch.Load(4)+1);
}


void touchScratchRenamed()
{
    touchScratch();
}
void touchScratchRenamed2()
{
    touchScratchRenamed();    
}

float4 foo(float b)
{
    touchScratchRenamed();
    return float4(b, b, b, b) + bar2(3);
}


[shader("raygeneration")]
void raygen_main()
{
    RayDesc MyRay = { float3(0,0,0),0.1,float3(1,0,0),1.0 };
    MyPayload payload = { float4(0,0,0,1) };
    TraceRay(MyAccelerationStructure, RAY_FLAG_NONE, 0xff, 0, 1, 0, MyRay, payload);
    RT[(unsigned int2)DispatchRaysIndex()] = payload.color + foo(1,1.5);
}

[shader("closesthit")]
void closesthit_main(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    touchScratchRenamed();
    payload.color.g = bar(5.5).y;    
}

[shader("anyhit")]
void anyhit_main(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    touchScratchRenamed2();
    payload.color += (0.1, 0.1, 0.1, 1);
}

[shader("miss")]
void miss_main(inout MyPayload payload)
{
    touchScratch();
    payload.color = (1, 0, 0, 1);
}