Texture2D diffuseMap : register(t0);
Texture2D diffuseMap_2 : register(t2);
Texture2D normalMap : register(t1);
SamplerState samLinear : register(s0);

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
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    nointerpolation uint texIndex : TEXCOORD3;
};

float3 ComputeTangent(float3 n)
{
    float3 tangent = float3(1, 0, 0);
    if (dot(n, tangent) > 0.99)
        tangent = float3(0, 0, 1);
    return normalize(tangent - n * dot(n, tangent));
}

float4 PSMain(PS_INPUT input) : SV_Target
{
    
    float4 diffuseColor;
    if (input.texIndex == 1)
    {
        diffuseColor = diffuseMap.Sample(samLinear, input.TexCoord);
        float3 normalSample = normalMap.Sample(samLinear, input.TexCoord).rgb * 2.0 - 1.0;
        float3 tangent = ComputeTangent(normalize(input.Normal));
        float3 bitangent = normalize(cross(normalize(input.Normal), tangent));
        float3x3 TBN = float3x3(tangent, bitangent, normalize(input.Normal));
        input.Normal = normalize(mul(normalSample, TBN));
    }
    else
    {
        diffuseColor = diffuseMap_2.Sample(samLinear, input.TexCoord);
    }

    float3 lightDir = normalize(lightPos - input.WorldPos);
    float lightDirLength = length(lightPos - input.WorldPos);
    float attenuation = clamp(1 / (lightDirLength * lightDirLength), 0, 1);

    float diff = saturate(dot(normalize(input.Normal), lightDir));
    float3 viewDir = normalize(cameraPosition - input.WorldPos);
    float3 reflectDir = reflect(-lightDir, normalize(input.Normal));
    float spec = pow(saturate(dot(viewDir, reflectDir)), 32);

    float3 ambientColor = ambient * diffuseColor.rgb;
    float3 diffuseColorFinal = diffuseColor.rgb * lightColor * diff * attenuation;
    float3 specularColor = lightColor * spec;

    float3 finalColor = ambientColor + diffuseColorFinal + specularColor;

    return float4(finalColor, diffuseColor.a);
}