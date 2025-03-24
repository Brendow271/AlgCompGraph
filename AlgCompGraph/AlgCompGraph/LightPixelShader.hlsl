cbuffer LightBuffer : register(b0)
{
    float3 lightPos;
    float pad0;
    float3 lightColor;
    float pad1;
    float3 ambient;
    float pad2;
    float3 cameraPosition;
    float pad3;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};

float4 PSMain(PS_INPUT input) : SV_Target
{
    return float4(lightColor, 1.0);
}