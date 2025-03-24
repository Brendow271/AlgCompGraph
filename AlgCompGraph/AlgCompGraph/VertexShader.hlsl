cbuffer ModelBuffer : register(b0)
{
    float4x4 model;
    float4x4 normal;
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
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 texcoord : TEXCOORD2;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(model,float4(input.position, 1.0));
    output.position = mul(vp, worldPos);
    output.worldPos = worldPos.xyz;
    output.normal = mul((float3x3) normal, input.normal);
    output.texcoord = input.texcoord;
    return output;
}