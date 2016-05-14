
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D pixels		: register(t0);
Texture2D mainTex		: register(t1);
SamplerState trilinear	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0,0,0,0);
	uint numSamples = 0;
	float2 uv = input.uv ;//+ float2(x * pixelWidth, y * pixelHeight);
	totalColor = mainTex.Sample(trilinear, uv)+ pixels.Sample(trilinear, uv);
	return totalColor ;
}