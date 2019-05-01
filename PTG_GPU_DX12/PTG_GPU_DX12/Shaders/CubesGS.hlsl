struct GSOutput
{
	float3 w_pos : POSITION;
	float3 color : COLOR;
};

void AppendQuad(float3 v0, float3 v1, float3 v2, float3 v3, inout TriangleStream<GSOutput> SO);

[maxvertexcount(36)] //6*6
void main(
	point float4 input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	float halfSize = 0.2f;
	//Defin  e the cubes corners 
	float3 c0 = float3(input[0].x - halfSize, input[0].y - halfSize, input[0].z - halfSize);
	float3 c1 = float3(input[0].x + halfSize, input[0].y - halfSize, input[0].z - halfSize);
	float3 c2 = float3(input[0].x - halfSize, input[0].y - halfSize, input[0].z + halfSize);
	float3 c3 = float3(input[0].x + halfSize, input[0].y - halfSize, input[0].z + halfSize);
		   																																  
	float3 c4 = float3(input[0].x - halfSize, input[0].y + halfSize, input[0].z - halfSize);
	float3 c5 = float3(input[0].x + halfSize, input[0].y + halfSize, input[0].z - halfSize);
	float3 c6 = float3(input[0].x - halfSize, input[0].y + halfSize, input[0].z + halfSize);
	float3 c7 = float3(input[0].x + halfSize, input[0].y + halfSize, input[0].z + halfSize);

	//Build the cube
	//bottom quad
	AppendQuad(c3, c2, c1, c0, output);
	//front quad
	AppendQuad(c0, c4, c1, c5, output);
	//back quad
	AppendQuad(c2, c3, c6, c7, output);
	//left side quad
	AppendQuad(c2, c6, c0, c4, output);
	//right side quad
	AppendQuad(c1, c5, c3, c7, output);
	//top quad
	AppendQuad(c4, c6, c5, c7, output);
}

void AppendQuad(float3 v0, float3 v1, float3 v2, float3 v3, inout TriangleStream<GSOutput> SO)
{
	GSOutput m_output;
	
	{
		m_output.w_pos = v0;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		m_output.w_pos = v1;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		m_output.w_pos = v2;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		SO.RestartStrip();

		m_output.w_pos = v2;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		m_output.w_pos = v1;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		m_output.w_pos = v3;
		m_output.color = float3(0.8f, 0.2f, 0.3f);
		SO.Append(m_output);
		SO.RestartStrip();
	}
}