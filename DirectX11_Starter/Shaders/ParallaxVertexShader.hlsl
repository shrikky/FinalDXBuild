
// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	matrix model;
	float3 lightPos;
	float3 viewPos;
	float3 dLightPos;
};
// Vertex data
struct VertexShaderInput
{ 
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;

};

// Output struct of Vertex shader
struct VertexToPixel
{	
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD1;
	float3 normal       : NORMAL;
	float3 TangentPLightPos	: TEXCOORD2;
	float3 TangentDLightPos	: TEXCOORD5;
    float3 TangentViewPos	: TEXCOORD3;
    float3 TangentFragPos	: TEXCOORD4;
};

VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;
	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(input.normal);
	float3 worldPos = mul(float4(input.position, 1.0f),world).xyz;
	output.uv = input.uv;


	//T B N is in world space
	float3 T = input.tangent;
    float3 N = input.normal;
	float3 B = cross(T,N);
    float3x3 TBN = float3x3(T, B, N);
	output.TangentPLightPos = mul(TBN , lightPos);
	output.TangentDLightPos = mul(TBN , dLightPos);
    output.TangentViewPos  =  mul(TBN , viewPos);
    output.TangentFragPos  = mul(TBN, worldPos);
	return output;

}