struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 PSMain(VSOutput pixel) : SV_Target0
{
    return float4(pixel.color.xyz, 0.5);
}