#include "MCTables.hlsli"

//This is a slightly modified version of Ludwig Pethrus Engström's implementation

struct Vertex
{
	float3 pos;
	float3 nor;
};

struct Triangle
{
	Vertex vertices[3];
};

RWStructuredBuffer<Triangle> vertexBuffer;
RWBuffer<uint> indexCounter; //Keeps track of how many triangles were written to the vertex buffer

Texture3D<float> densityTexture : register(t0);
SamplerState LinearClamp : register(s0);

cbuffer PerChunkData : register(b1)
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
// since our local coords go from (0,0,0) lower-left to (1,1,1) upper-right we must invert the y for texture coordinatse
float3 localToTextureCoord(float3 localCoord)
{
	localCoord.y = 1 - localCoord.y;
	return localCoord;
}

// This function interpolates a vertex position between p1 and p2 with density values d1 and d2 respectivly
float3 CalculateVertex(float3 p1, float3 p2, float d1, float d2)
{
	float3 dir = (p2 - p1);
	float s = -d1 / (d2 - d1);
	return p1 + s * dir; 

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

[numthreads(8,8,8)]
void main(uint3 id : SV_DispatchThreadID)
{
	float cellDensity[8];	// density values at each corner of the voxel/cell (local to each thread)
	float3 localCoords[8]; // local coordinates for each corner within the chunk (local to each thread)

	// lower left corners local coordinate
	float3 localCoordLL = (float3)(id * invVoxelDim);
	localCoords[0] = localCoordLL;																					// v0 - lower left
	localCoords[1] = localCoordLL + float3(0, invVoxelDim, 0);										// v1 - upper left
	localCoords[2] = localCoordLL + float3(invVoxelDim, invVoxelDim, 0);						// v2 - upper right
	localCoords[3] = localCoordLL + float3(invVoxelDim, 0, 0);										// v3 - lower right

	localCoords[4] = localCoordLL + float3(0, 0, invVoxelDim);										// v4 - lower back left
	localCoords[5] = localCoordLL + float3(0, invVoxelDim, invVoxelDim);						// v5 - upper back left
	localCoords[6] = localCoordLL + float3(invVoxelDim, invVoxelDim, invVoxelDim);	// v6 - upper back right
	localCoords[7] = localCoordLL + float3(invVoxelDim, 0, invVoxelDim);						// v7 - lower back right

	int caseNumber = 0;
	for (int i = 0; i < 8; i++)
	{
		float3 sampCoord = localToTextureCoord(localCoords[i]);
		cellDensity[i] = densityTexture.SampleLevel(LinearClamp, sampCoord, 0);
		if (cellDensity[i] >= 0) caseNumber |= 1 << i;
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
			float3 pLocal = CalculateVertex(p1Local, p2Local, cellDensity[v1], cellDensity[v2]);
			float3 normal = CalculateNormal(localToTextureCoord(pLocal));
			normal.y *= -1;

			// convert from local to world coordinates
			Vertex vert;
			vert.pos = localToWorldCoord(pLocal);
			//vert.vertPos = pLocal;
			vert.nor= normal;
			t.vertices[e] = vert;
		}
		
		uint index = 0;
		InterlockedAdd(indexCounter[0], 1, index);
		vertexBuffer[index] = t;
	}
}