struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer GeomBuffer : register(b0)
{
    float4x4 model;
};

cbuffer ViewBuffer : register(b1)
{
    float4x4 view;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 WorldPos = float4(input.position, 1.0f);
    output.position = mul(WorldPos, model);
    output.position = mul(view, output.position);
    output.uv = input.uv;
    return output;
}