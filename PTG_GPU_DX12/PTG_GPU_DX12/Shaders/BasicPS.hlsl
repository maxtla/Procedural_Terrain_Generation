
struct PxlInput
{
    float4 sv_pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PxlInput input) : SV_TARGET
{
    return input.color;
}