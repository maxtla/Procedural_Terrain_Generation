
struct PxlInput
{
    float4 sv_pos : SV_POSITION;
    float4 color : COLOR;
};

float4 PSMain(PxlInput input) : SV_TARGET
{
	return float4(0.33441f, 0.3f, 0.122344f, 1.0f);
    //return input.color;
}