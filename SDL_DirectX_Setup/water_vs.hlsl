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
    float time;
};

float SumOfSinesApproximation(float2 xzpos)
{
    // Waves
    float L = 5; // wavelength
    float W = 2.0 / L; // freq W = 2 / L
    float A = 1; // amplitude
    float S = 1.5; // Speed crest moves per second in units
    float2 D = float2(1, -1); // wave direction on xz, 2d plane
    float PSI = S * 2 / L;
    
    return A * sin(dot(D, xzpos) * W + time * PSI);
}

float2 DirectionalWave(float2 xzpos)
{
    return xzpos / abs(xzpos);

}

OutputVertex main(InputVertex input)
{
    OutputVertex output;
    
    output.color = input.color;

    output.position = float4(input.position, 1);
    
    output.position.y = SumOfSinesApproximation(output.position.xz);
    //output.position.y = DirectionalWave(output.position.xz);
    
    output.position = mul(output.position, worldmat);
    output.worldpos = output.position.xyz;
    output.position = mul(output.position, viewmat);
    output.position = mul(output.position, projectionMat);
    
    return output;
}