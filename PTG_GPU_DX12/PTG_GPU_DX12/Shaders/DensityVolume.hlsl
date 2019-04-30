
RWTexture3D<float> densityBuffer : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 id : SV_DispatchThreadID )
{
	//have some density function here to calculate the corner densities of the voxels in the chunk
	//here we have a chunk that has 8*8*8 (512) density corners
	float density = sin(id.x) * cos(id.y) * sin(id.z);

	if (density < 0.5f)
		density = 0.0f;

	densityBuffer[id] = density;
}