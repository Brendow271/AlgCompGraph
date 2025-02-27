TextureCube colorTexture : register(t0);
SamplerState colorSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float3 localPos : POSITION1;
};

float4 PSMain(VSOutput pixel) : SV_Target0
{
    return float4(colorTexture.Sample(colorSampler, pixel.localPos).xyz, 1.0);
}