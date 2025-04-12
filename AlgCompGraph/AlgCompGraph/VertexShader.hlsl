#define NUM_INSTANCES 10
#define NUM_TEX 2

cbuffer ModelBuffer : register(b0)
{
    float4x4 models[NUM_INSTANCES];
    float4x4 normals[NUM_INSTANCES];
    float4 isNormalMapActive[NUM_INSTANCES];
};

cbuffer VPBuffer : register(b1)
{
    float4x4 vp;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    uint instanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 texcoord : TEXCOORD2;
    nointerpolation uint texIndex : TEXCOORD3;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    uint instanceID = input.instanceID;
    float4 worldPos = mul(models[instanceID], float4(input.position, 1.0));
    output.position = mul(vp, worldPos);
    output.worldPos = worldPos.xyz;
    output.normal = mul((float3x3) normals[instanceID], input.normal);
    output.texcoord = input.texcoord;
    output.texIndex = isNormalMapActive[instanceID].x > 0 ? 1 : 0;
    return output;
}