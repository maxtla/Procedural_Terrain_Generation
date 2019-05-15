#include "MCTables.hlsli"

//This is a modified version of Ludwig Pethrus Engström's implementation

struct Vertex
{
	float3 pos;
	float3 nor;
};

struct Triangle
{
	Vertex vertices[3];
};

RWStructuredBuffer<Triangle> vertexBuffer : register(u0);

Texture3D<float> densityTexture : register(t0);
SamplerState LinearClamp : register(s0);

cbuffer PerDispatchData : register(b2)
{
	uint ThreadGroups_X;
	uint ThreadGroups_Y;
	uint ThreadGroups_Z;
	uint NumThreadsPerGroup;
	float isoValue;
};

cbuffer PerChunkData : register(b1) //I need to fix DescriptorHeap so everything can be bound from the same heap
{
	float invVoxelDim;
	float3 wsChunkPosLL;
	float3 wsChunkDim;
	int voxelDim;
};

float3 localToWorldCoord(float3 localPos)
{
	return wsChunkPosLL + (localPos * wsChunkDim);
}

float3 CalculateNormal(uint3 index) 
{
	float3 gradient;
	gradient.x = densityTexture.Load(float4(float3(index.x + 1, index.y, index.z), 0)) -
		densityTexture.Load(float4(float3(index.x - 1, index.y, index.z), 0));
	gradient.y = densityTexture.Load(float4(float3(index.x, index.y + 1, index.z), 0)) -
		densityTexture.Load(float4(float3(index.x, index.y - 1, index.z), 0));
	gradient.z = densityTexture.Load(float4(float3(index.x, index.y, index.z + 1), 0)) -
		densityTexture.Load(float4(float3(index.x, index.y, index.z - 1), 0));
	return -normalize(gradient);
}
// This function interpolates a vertex position between p1 and p2 with density values d1 and d2 respectivly
Vertex CalculateVertex(float3 p1, float3 p2, float d1, float d2, float isoValue)
{
	float3 dir = (p2 - p1);
	float s = ( isoValue - d1) / (d2 - d1);
	float3 pos = p1 + s * dir; 

	float3 nor1 = CalculateNormal(uint3(p1));
	float3 nor2 = CalculateNormal(uint3(p2));
	float3 nor = nor1 + s * (nor2 - nor1); //interpolate the normal to match the interpolated position

	Vertex v;
	v.pos = pos;
	v.nor = normalize(nor);

	return v;
}

void CalculateNormals(inout Triangle t)
{
	float3 edge1 = t.vertices[1].pos - t.vertices[0].pos;
	float3 edge2 = t.vertices[2].pos - t.vertices[0].pos;
	float3 normal = normalize(cross(edge1, edge2));

	t.vertices[0].nor = normal;
	t.vertices[1].nor = normal;
	t.vertices[2].nor = normal;
}

//largest possible product is 10*10*10 = 1000 since max threads per group is 1024 for shader model 5.0
//Keep  it at a multiple of 8, this is good for the different architectures (Nvidia, AMD and Intel) 
[numthreads(8,8,8)]
void main(uint3 id : SV_DispatchThreadID)
{
	uint border_X = ThreadGroups_X * NumThreadsPerGroup;
	uint border_Y = ThreadGroups_Y * NumThreadsPerGroup;
	uint border_Z = ThreadGroups_Z * NumThreadsPerGroup;
	if (id.x >= border_X || id.y >= border_Y || id.z >= border_Z)
		return;

	float cellDensity[8];	// density values at each corner of the voxel/cell (local to each thread)
	float3 localCoords[8]; // local coordinates for each corner within the chunk (local to each thread)

	float3 corners[8];
	corners[0] = float3(id);									//bottom left front		v0
	corners[1] = corners[0] + float3(0, 1, 0);		//top left front				v1
	corners[2] = corners[0] + float3(1, 1, 0);		//top right front			v2
	corners[3] = corners[0] + float3(1, 0, 0);		//bottom right front	v3

	corners[4] = corners[0] + float3(0, 0, 1);		//bottom left back		v4
	corners[5] = corners[0] + float3(0, 1, 1);		//top left back				v5
	corners[6] = corners[0] + float3(1, 1, 1);		//top right back			v6
	corners[7] = corners[0] + float3(1, 0, 1);		//bottom right back		v7

	int caseNumber = 0;
	for (int i = 0; i < 8; i++)
	{
		localCoords[i] = corners[i];
		int3 index = int3(corners[i]);
		cellDensity[i] = densityTexture.Load(int4(index, 0)).r;
		if (cellDensity[i] > isoValue) caseNumber |= 1 << i;
	}

	int numPolys = 0;
	numPolys = case_to_numpolys[caseNumber]; //use the case number on the look up tables to retrieve the nr of triangles to be created

	for (int n = 0; n < numPolys; n++)
	{
		Triangle t;
		// go through the three edges that has been cut and calculate a vertex position for where it was cut
		for (int e = 0; e < 3; e++)
		{

			//Get the edge connect list with case number and the index and then get the individual component of the list
			int edgeNum = edge_connect_list[caseNumber][n][e];
			int v1 = edge_list[edgeNum].x; // get the vertices connected by the edge
			int v2 = edge_list[edgeNum].y;

			// get local position of the vertices 
			float3 p1Local = localCoords[v1];
			float3 p2Local = localCoords[v2];

			// linearly interpolate vertex between p1 and p2
			Vertex pLocal = CalculateVertex(p1Local, p2Local, cellDensity[v1], cellDensity[v2], isoValue);
			// convert from local to world coordinates
			t.vertices[e] = pLocal;
		}

		//CalculateNormals(t); //Use this if we want vertex shading, otherwise "soft" shading is used
		vertexBuffer[vertexBuffer.IncrementCounter()] = t;
	}
}