struct GSOutput
{
	float4 w_pos : POSITION;
	float4 color : COLOR;
};

void AppendQuad(float4 v0, float4 v1, float4 v2, float4 v3, inout TriangleStream<GSOutput> SO);

[maxvertexcount(36)] //6*6
void main(
	point float4 input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	float halfSize = 0.2f;
	//Define the cubes corners 
	float4 c0 = float4(input[0].x - halfSize, input[0].y - halfSize, input[0].z - halfSize, 1.0f);
	float4 c1 = float4(input[0].x + halfSize, input[0].y - halfSize, input[0].z - halfSize, 1.0f);
	float4 c2 = float4(input[0].x - halfSize, input[0].y - halfSize, input[0].z + halfSize, 1.0f);
	float4 c3 = float4(input[0].x + halfSize, input[0].y - halfSize, input[0].z + halfSize, 1.0f);

	float4 c4 = float4(input[0].x - halfSize, input[0].y + halfSize, input[0].z - halfSize, 1.0f);
	float4 c5 = float4(input[0].x + halfSize, input[0].y + halfSize, input[0].z - halfSize, 1.0f);
	float4 c6 = float4(input[0].x - halfSize, input[0].y + halfSize, input[0].z + halfSize, 1.0f);
	float4 c7 = float4(input[0].x + halfSize, input[0].y + halfSize, input[0].z + halfSize, 1.0f);

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

void AppendQuad(float4 v0, float4 v1, float4 v2, float4 v3, inout TriangleStream<GSOutput> SO)
{
	GSOutput m_output;
	
	{
		m_output.w_pos = v0;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		m_output.w_pos = v1;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		m_output.w_pos = v2;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		SO.RestartStrip();

		m_output.w_pos = v2;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		m_output.w_pos = v1;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		m_output.w_pos = v3;
		m_output.color = float4(0.8f, 0.2f, 0.3f, 1.0f);
		SO.Append(m_output);
		SO.RestartStrip();
	}
}