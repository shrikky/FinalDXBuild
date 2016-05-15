
Texture2D diffuseTexture	: register(t0);
TextureCube skyTexture	: register(t2);

SamplerState trilinear	: register(s0);
SamplerComparisonState shadowSampler : register(s1);
Texture2D normalMap			:register(t1);

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos     : TEXCOORD0;
	float2 uv           : TEXCOORD1;
	float4 posForShadow : TEXCOORD2;
};
struct DirectionalLight {
	float4 Color;		// 4
	float3 Direction;			// 3

};
struct PointLight {
	float4 PointLightColor;		// 7
	float3 Position;			// 3
};
struct SpecularLight {
	float4 SpecularColor;		// 4			
	float SpecularStrength;		// 1
};
cbuffer externalData : register(b0)
{
	DirectionalLight directionLight;
	PointLight pointLight;
	SpecularLight specularLight;
	float3 cameraPosition;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	float2 texCoords = input.uv;
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));		// This is improvement code, we can also use T = normalize(input.tangent)
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Sample the normal map
	float3 normalFromMap = normalMap.Sample(trilinear, input.uv).rgb;

	// Unpack the normal
	normalFromMap = normalFromMap * 2 - 1;

	// Adjust and overwrite the existing normal
	input.normal = normalize(mul(normalFromMap, TBN));

	// Light direction
	float3 lightDir = normalize(directionLight.Direction);

	// Basic diffuse calculation (n dot l)
	// We need the direction from the surface TO the light
	float dirNdotL = saturate(dot(input.normal, -lightDir));

	// Point light --------------------------------------------

	// Get direction to the point light
	float3 dirToPointLight = normalize(pointLight.Position - input.worldPos);
	float pointNdotL = saturate(dot(input.normal, dirToPointLight));

	// Point light specular ----------------------------------
	float3 dirToCamera = normalize(cameraPosition - input.worldPos);
	float3 refl = reflect(-dirToPointLight, input.normal);
	float spec = pow(max(dot(refl, dirToCamera), 0), 64.0f);

	//float3 surfaceColor = diffuseTexture.Sample(trilinear, texCoords).rgb;

	// Grab the diffuse color
	float4 diffuseColor = diffuseTexture.Sample(trilinear, input.uv);

	float4 pointLightColor = float4(1, 1, 1, 1);

	// Calculate the reflection vector and sample the sky
	float4 reflectionColor = skyTexture.Sample(
		trilinear,
		reflect(-dirToCamera, input.normal));

	// Combine lights
	float4 surfaceColor = (pointLightColor * pointNdotL * diffuseColor) + (directionLight.Color * dirNdotL * diffuseColor) + float4(spec.xxx, 1);

	// Combine the reflection and surface
	return lerp(reflectionColor, surfaceColor, 0.5f);
}