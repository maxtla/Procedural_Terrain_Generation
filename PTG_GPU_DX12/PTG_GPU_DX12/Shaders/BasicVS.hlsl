
cbuffer ViewProj : register(b0)
{
    matrix viewM;
    matrix projM;
};

struct VtxInput
{
    float3 w_pos : POSITION;
    float3 normal : NORMAL;
};

struct VtxOutput
{
    float4 sv_pos : SV_POSITION;
    float4 normal  : NORMAL;
};

VtxOutput VSMain(VtxInput input)
{
    VtxOutput output;
	float4 w_pos = float4(input.w_pos, 1.0f);

    output.sv_pos = mul(w_pos, viewM);
    output.sv_pos = mul(output.sv_pos, projM);

	output.normal = float4(input.normal, 1.0f);

    return output;
}
