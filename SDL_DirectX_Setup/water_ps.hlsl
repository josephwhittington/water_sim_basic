struct OutputVertex
{
    float4 position : SV_Position;
    float3 worldpos : WORLDPOS;
    float3 color : COLOR;
};

float4 main(OutputVertex input) : SV_TARGET
{
    float pos = (input.worldpos.y + 1) / 2;
    //return float4(0, 1 - pos, pos, 1);
    return float4(input.color, 1.0f);
}