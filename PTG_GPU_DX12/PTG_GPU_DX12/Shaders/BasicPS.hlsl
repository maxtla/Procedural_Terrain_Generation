
struct PxlInput
{
    float4 sv_pos : SV_POSITION;
    float4 normal : NORMAL;
};


float4 PSMain(PxlInput input) : SV_TARGET
{
	float4 lightDir = float4(0.5f, 5.f, -0.5f, 1.0f);
	float lambda = dot(normalize(input.normal), normalize(lightDir));
	float4 color = float4(0.3f * lambda, 0.4f * lambda, 0.8f * lambda, 1.0f);
	return color;
}