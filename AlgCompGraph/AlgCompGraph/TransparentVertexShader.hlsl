cbuffer ModelBuffer : register(b0)
{
    matrix model;
    matrix normal;
};

cbuffer VPBuffer : register(b1)
{
    matrix viewProj;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(model, float4(input.position, 1.0));
    output.position = mul(viewProj, worldPos);
    output.worldPos = worldPos.xyz;
    output.normal = mul((float3x3) normal, input.normal);
    return output;
}