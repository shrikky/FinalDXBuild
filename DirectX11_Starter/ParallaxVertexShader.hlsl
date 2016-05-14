
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix model;
	float3 lightPos;
	float3 viewPos;
};
// Vertex data
struct VertexShaderInput
{ 
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float2 uv			: TEXCOORD;
};

// Output struct of Vertex shader
struct VertexToPixel
{	

	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos     : TEXCOORD0;
	float3 TangentLightPos	: TEXCOORD2;
    float3 TangentViewPos	: TEXCOORD3;
    float3 TangentFragPos	: TEXCOORD4;
	float2 uv           : TEXCOORD1;
	// RGBA color
};

VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = mul(input.normal, (float3x3)world );
	output.tangent = mul(input.tangent, (float3x3)world);
	output.normal = normalize(input.normal);
	output.worldPos = mul(float4(input.position, 1.0f),world).xyz;
	output.uv = input.uv;
	    
    float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));		
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	output.TangentLightPos = mul(TBN , lightPos);
    output.TangentViewPos  =  mul(TBN , viewPos);
    output.TangentFragPos  = mul(TBN, output.position);

	return output;
}