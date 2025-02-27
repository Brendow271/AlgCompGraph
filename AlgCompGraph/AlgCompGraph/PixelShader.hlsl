Texture2D colorTexture : register(t0);
SamplerState colorSampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_Target
{
    return colorTexture.Sample(colorSampler, input.uv);
}