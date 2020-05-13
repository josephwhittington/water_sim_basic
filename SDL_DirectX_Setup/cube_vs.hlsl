#pragma pack_matrix(row_major)

struct InputVertex
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct OutputVertex
{
    float4 position : SV_Position;
    float3 worldpos : WORLDPOS;
    float3 color : COLOR;
};

cbuffer WORLD : register(b0)
{
    float4x4 worldmat;
    float4x4 viewmat;
    float4x4 projectionMat;
};

OutputVertex main(InputVertex input)
{
    OutputVertex output;
    
    output.color = input.color;

    output.position = float4(input.position, 1);
    output.position = mul(output.position, worldmat);
    output.worldpos = output.position.xyz;
    output.position = mul(output.position, viewmat);
    output.position = mul(output.position, projectionMat);
    
    return output;
}