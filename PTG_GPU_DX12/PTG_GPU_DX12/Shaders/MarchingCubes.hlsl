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

// This function interpolates a vertex position between p1 and p2 with density values d1 and d2 respectivly
float3 CalculateVertex(float3 p1, float3 p2, float d1, float d2, float isoValue)
{
	float3 dir = (p2 - p1);
	float s = (isoValue-d1) / (d2 - d1);
	return p1 + s * dir; 
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

float3 CalculateNormal(float3 uvw)
{
	float d;
	d = invVoxelDim;
	float3 gradient;
	gradient.x = densityTexture.SampleLevel(LinearClamp, uvw + float3(d, 0, 0), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(-d, 0, 0), 0).x;
	gradient.y = densityTexture.SampleLevel(LinearClamp, uvw + float3(0, d, 0), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(0, -d, 0), 0).x;
	gradient.z = densityTexture.SampleLevel(LinearClamp, uvw + float3(0, 0, d), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(0, 0, -d), 0).x;

	return -normalize(gradient);
}

[numthreads(2,2,2)]
void main(uint3 id : SV_DispatchThreadID)
{
	int border = 8;
	if (id.x == border || id.y == border || id.z == border)
		return;

	float cellDensity[8];	// density values at each corner of the voxel/cell (local to each thread)
	float3 localCoords[8]; // local coordinates for each corner within the chunk (local to each thread)

	float3 corners[8];
	corners[0] = float3(id);									//top left front
	corners[1] = corners[0] + float3(1, 0, 0);		//top right front
	corners[2] = corners[0] + float3(0, 1, 0);		//bottom left front
	corners[3] = corners[0] + float3(1, 1, 0);		//bottom right front

	corners[4] = corners[0] + float3(0, 0, 1);		//top left back
	corners[5] = corners[0] + float3(1, 0, 1);		//top right back
	corners[6] = corners[0] + float3(0, 1, 1);		//bottom left back
	corners[7] = corners[0] + float3(1, 1, 1);		//bottom right back

	float isoLevel = 0.0f;
	int caseNumber = 0;
	for (int i = 0; i < 8; i++)
	{
		localCoords[i] = corners[i] * invVoxelDim;
		int3 index = int3(corners[i]);
		cellDensity[i] = densityTexture.Load(int4(index, 0)).r;
		if (cellDensity[i] <= isoLevel) caseNumber |= 1 << i;
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
			float3 pLocal = CalculateVertex(p1Local, p2Local, cellDensity[v1], cellDensity[v2], isoLevel);
			// convert from local to world coordinates
			Vertex vert;
			vert.pos = localToWorldCoord(pLocal);
			t.vertices[e] = vert;
		}
		CalculateNormals(t);
		vertexBuffer[vertexBuffer.IncrementCounter()] = t;
	}
}