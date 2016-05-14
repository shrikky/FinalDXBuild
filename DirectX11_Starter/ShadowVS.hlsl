

// Constant buffer for C++ data being passed in
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Describes individual vertex data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float4 posForShadow : TEXCOORD2;
};


float4 main(VertexShaderInput input) : SV_POSITION
{
	// Calculate output position
	matrix worldViewProj = mul(mul(world, view), projection);
	return mul(float4(input.position, 1.0f), worldViewProj);
}