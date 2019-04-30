#include "MCTables.hlsli"

//This is a modified version of Ludwig Pethrus Engstr�m's implementation

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
//RWBuffer<uint> indexCounter : register(u5); //Keeps track of how many triangles were written to the vertex buffer

Texture3D<float> densityTexture : register(t0);
SamplerState LinearClamp : register(s0);

cbuffer Placeholder : register(b0)
{
	matrix pl;
	matrix pl1;
}

cbuffer PerChunkData : register(b1) //I need to fix DescriptorHeap so everything can be bound from the same heap
{
	float invVoxelDim0;
	float3 wsChunkPosLL0;
	float3 wsChunkDim0;
	int voxelDim0;
};
//Temporary "CB"
static float invVoxelDim = 1.f/8.0f;
static float3 wsChunkPosLL = float3(0.0f, 0.0f, 0.0f);
static float3 wsChunkDim = float3(1.0f, 1.0f, 1.0f);
static int voxelDim = 8;

float3 localToWorldCoord(float3 localPos)
{
	return wsChunkPosLL + (localPos * wsChunkDim);
}

// This function interpolates a vertex position between p1 and p2 with density values d1 and d2 respectivly
float3 CalculateVertex(float3 p1, float3 p2, float d1, float d2)
{
	float3 dir = (p2 - p1);
	float s = -d1 / (d2 - d1);
	return p1 + s * dir; 

}

float3 CalculateNormal(Triangle t)
{
	float3 edge1 = t.vertices[1].pos - t.vertices[0].pos;
	float3 edge2 = t.vertices[2].pos - t.vertices[0].pos;
	return normalize(cross(edge1, edge2));
}

//float3 CalculateNormal(float3 uvw)
//{
//	float d;
//	d = invVoxelDim;
//	float3 gradient;
//	gradient.x = densityTexture.SampleLevel(LinearClamp, uvw + float3(d, 0, 0), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(-d, 0, 0), 0).x;
//	gradient.y = densityTexture.SampleLevel(LinearClamp, uvw + float3(0, d, 0), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(0, -d, 0), 0).x;
//	gradient.z = densityTexture.SampleLevel(LinearClamp, uvw + float3(0, 0, d), 0).x - densityTexture.SampleLevel(LinearClamp, uvw + float3(0, 0, -d), 0).x;
//
//	return -normalize(gradient);
//}

[numthreads(1,1,1)]
void main(uint3 id : SV_DispatchThreadID)
{

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

	int caseNumber = 0;
	for (int i = 0; i < 8; i++)
	{
		localCoords[i] = corners[i] * invVoxelDim;
		int3 index = int3(corners[i]);
		//cellDensity[i] = densityTexture.Load(int4(index, 0)).r;

		corners[i].x = corners[i].x / (float)(voxelDim + 1);
		corners[i].y = corners[i].y / (float)(voxelDim + 1);
		corners[i].z = corners[i].z / (float)(voxelDim + 1);
		cellDensity[i] = densityTexture.SampleLevel(LinearClamp, corners[i], 0);

		if (cellDensity[i] >= 0) caseNumber |= 1 << i;
	}

	int numPolys = 0;
	numPolys = case_to_numpolys[caseNumber]; //use the case number on the look up tables to retrieve the nr of triangles to be created

	Triangle test;
	test.vertices[0].pos = float3(id); test.vertices[0].nor = float3(caseNumber, numPolys, cellDensity[0]);
	test.vertices[1].pos = float3(cellDensity[1], cellDensity[2], cellDensity[3]); test.vertices[1].nor = float3(cellDensity[4], cellDensity[5], cellDensity[6]);
	test.vertices[2].pos = float3(cellDensity[7], -1, -1); test.vertices[2].nor = float3(-1, -1, -1);

	vertexBuffer[vertexBuffer.IncrementCounter()] = test;
	return;

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
			float3 pLocal = CalculateVertex(p1Local, p2Local, cellDensity[v1], cellDensity[v2]);

			// convert from local to world coordinates
			Vertex vert;
			vert.pos = localToWorldCoord(pLocal);
			//vert.vertPos = pLocal;
			t.vertices[e] = vert;
		}

		float3 normal = CalculateNormal(t);
		//normal.y *= -1;
		t.vertices[0].nor = normal;
		t.vertices[1].nor = normal;
		t.vertices[2].nor = normal;

		vertexBuffer[vertexBuffer.IncrementCounter()] = t;
	}
}