struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
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
    float4 WorldPos = float4(input.pos, 1.0f);
    output.pos = mul(model, WorldPos);
    output.pos = mul(view, output.pos);
    output.color = input.color;
    return output;
}