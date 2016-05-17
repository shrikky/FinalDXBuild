
Texture2D diffuseTexture	: register(t0);


SamplerState trilinear	: register(s0);
SamplerComparisonState shadowSampler : register(s1);
Texture2D normalMap			:register(t1);
Texture2D depthMap			:register(t2);
Texture2D shadowMap		: register(t3);

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
	float4 AmbientColor;		// 4
	float4 DiffuseColor;		// 4
	float3 Direction;			// 3

};
struct PointLight {
	float4 PointLightColor;		// 7
	float3 Position;			// 3
	float  Strength;
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
// Directional Light calculation
float4 CalculateDirectionalLight(float3 normal, DirectionalLight light) {
	float3 output;
	normal = normalize(normal);
	float NdotL = saturate(dot(normal, directionLight.Direction));
	output = (light.DiffuseColor * NdotL).xyz;
	output += light.AmbientColor.xyz;
	return float4(output, 1);

}
// PointLightCalculation
float CalculatePointLight(float3 normal, float3 direction, float dist) {

	float point_NdotL;
	point_NdotL = saturate(dot(normal, direction));
	point_NdotL = mul(point_NdotL, pointLight.Strength);
	return point_NdotL;

}
///SpecularLight Calculation
float SpecLight(float3 normal, float3 camDir, float3 lightTowardsPLight, float strength) {
	float spec;
	//Phong reflection
	//float3 reflection = reflect(-lightTowardsPLight, normal);
	//spec = pow(max(dot(reflection, camDir),0),32);
	
	//Blinn-Phong shading model

	float3 halfway = normalize(-lightTowardsPLight + camDir); //halfway vector

	spec = pow(max(dot(halfway, camDir), 0), 256);

	return spec * strength;

}
float2 ParallaxMapping(float2 texCoords, float3 viewDir){

	const float minLayers = 10;
	const float maxLayers = 20;
	float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDir)));  

	float layerDepth  = 1.0f/numLayers;
	float currentLayerDepth  = 0;

	float2 p = viewDir.xy / viewDir.z * 0.1f;
	return texCoords - p;
	float2 deltaTexCoords  = p/numLayers;
	float2 currentTexCoords  = texCoords;
	float currentDepthMapValue  = depthMap.Sample(trilinear, currentTexCoords).r;

	[unroll(100)]while(currentLayerDepth < currentDepthMapValue)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = depthMap.Sample(trilinear, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}
	// get texture coordinates before collision (reverse operations)
		float2 prevTexCoords = currentTexCoords + deltaTexCoords;

		// get depth after and before collision for linear interpolation
		float afterDepth  = currentDepthMapValue - currentLayerDepth;
		float beforeDepth =  depthMap.Sample(trilinear, prevTexCoords).r - currentLayerDepth + layerDepth;
 
		// interpolation of texture coordinates
		float weight = afterDepth / (afterDepth - beforeDepth);
		float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
		//return finalTexCoords;   
}

float3 CalculateNormalMap(inout float3 dirTowardsCamera, float3x3 TBN, inout float2 texCoords, float3 normalFromMap) 
{	


	texCoords =  ParallaxMapping(texCoords,dirTowardsCamera);
	   // discards a fragment when sampling outside default texture region (fixes border artifacts)

	normalFromMap = normalMap.Sample(trilinear, texCoords).rgb;
	normalFromMap = normalFromMap * 2 - 1;	// Normal unpacking

	return normalFromMap;
}


float4 main(VertexToPixel input) : SV_TARGET
{	
	input.normal = normalize(input.normal);
	float2 texCoords = input.uv;
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));		// This is improvement code, we can also use T = normalize(input.tangent)
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);


	float3 dirTowardsCamera = normalize(cameraPosition - input.worldPos);
	dirTowardsCamera = mul(dirTowardsCamera,TBN);
	float3 output;
	float3 normalFromMap;
	normalFromMap = CalculateNormalMap( dirTowardsCamera, TBN, texCoords, normalFromMap);
	input.normal = normalize(mul(normalFromMap, TBN));
	float dist = distance(pointLight.Position,input.worldPos);
	float3 dirTowardsPointLight = normalize(pointLight.Position - input.worldPos);
	
	// Calculate shadow amount

	// Calculate this pixel's UV coordinate ON THE SHADOW MAP
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;

	// Flip the Y value (since texture coords and clip space are opposite)
	shadowUV.y = 1 - shadowUV.y;

	// Calculate this pixel's actual depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample the shadow map and (automatically) compare the values
	float shadowAmount = shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, depthFromLight);


	float3 surfaceColor = diffuseTexture.Sample(trilinear, texCoords).rgb;

	output =	pointLight.PointLightColor * CalculatePointLight(input.normal, dirTowardsPointLight, dist) +
	 float4(SpecLight(input.normal, dirTowardsCamera, dirTowardsPointLight, specularLight.SpecularStrength).xxx, 0);
	//+ CalculateDirectionalLight(input.normal, directionLight);
	return float4(output, 1) * float4(surfaceColor,1) * shadowAmount;
}