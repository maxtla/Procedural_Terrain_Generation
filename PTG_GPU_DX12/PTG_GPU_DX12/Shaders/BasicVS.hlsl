
cbuffer ViewProj : register(b0)
{
    matrix viewM;
    matrix projM;
};

struct VtxInput
{
    float4 w_pos : POSITION;
    float4 color : COLOR;
};

struct VtxOutput
{
    float4 sv_pos : SV_POSITION;
    float4 color  : COLOR;
};

VtxOutput VSMain(VtxInput input)
{
    VtxOutput output;

    output.sv_pos = mul(input.w_pos, viewM);
    output.sv_pos = mul(output.sv_pos, projM);

	output.color = input.color;

    return output;
}
