cbuffer VPBuffer : register(b0)
{
    float4x4 vp;
};

cbuffer GeomBuffer : register(b1)
{
    float4x4 model;
    float4 color;
};

struct VSInput
{
    float3 pos : POSITION;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    output.pos = mul(vp, worldPos);
    output.color = color;
    return output;
}