
cbuffer ViewProj : register(b0)
{
    float4x4 viewM;
    float4x4 projM;
};

cbuffer WorldMatrix : register(b1)
{
    float4x4 worldM;
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

    output.sv_pos = mul(input.w_pos, worldM);
    output.sv_pos = mul(output.sv_pos, viewM);
    output.sv_pos = mul(output.sv_pos, projM);

    output.color = input.color;

    return output;
}
