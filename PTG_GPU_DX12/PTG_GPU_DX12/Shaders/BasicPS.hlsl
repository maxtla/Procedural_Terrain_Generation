
struct PxlInput
{
    float4 sv_pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PxlInput input) : SV_TARGET
{
    return saturate(float4(sin(input.sv_pos.x), 0.5f, 0.5f, 0.5f));
}