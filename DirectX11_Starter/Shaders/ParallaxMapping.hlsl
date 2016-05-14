Texture2D diffuseTexture	: register(t0);
Texture2D normalMap			:register(t1);
Texture2D depthMap			:register(t2);
SamplerState trillinear	: register(s0);

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
struct DirectionalLight {
	float4 AmbientColor;		
	float4 DiffuseColor;		
	float3 Direction;		

};
struct PointLight {
	float4 PointLightColor;		
	float3 Position;	
	float  Strength;		
};
struct SpecularLight {
	float4 SpecularColor;					
	float SpecularStrength;		
};
cbuffer externalData : register(b0)
{
	DirectionalLight directionLight;
	PointLight pointLight;
	SpecularLight specularLight;
};
// Directional Light calculation
float4 CalculateDirectionalLight(float3 normal, DirectionalLight light, float3 tangentLightPos) {
	
	float3 output;

	normal = normalize(normal);
	float NdotL = saturate(dot(normal, tangentLightPos));
	output = light.DiffuseColor * NdotL;
	output += light.AmbientColor;
	return float4(output, 1);

}
// PointLightCalculation
float CalculatePointLight(float3 normal, float3 direction, float dist) {

	float point_NdotL;
	normal = normalize(normal);
	point_NdotL = saturate(dot(normal, direction));
	point_NdotL = mul(point_NdotL, pointLight.Strength);
	return point_NdotL;

}
///SpecularLight Calculation
float SpecLight(float3 normal, float3 camDir, float3 lightTowardsPLight, float strength) {
	float spec;
	//Blinn-Phong shading model
	float3 halfway = normalize(lightTowardsPLight + camDir); //halfway vector
	spec = pow(max(dot(halfway, camDir), 0), 16);
	return spec * strength;

}
float2 ParallaxMapping(float2 texCoords, float3 viewDir){

	float height =  depthMap.Sample(trillinear,texCoords).r;    
    float2 p = viewDir.xy * (height * 0.1f);
    return texCoords - p;   
}
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 output;
	float3 viewDir = normalize(input.TangentViewPos - input.TangentFragPos);
	float2 texCoords =  ParallaxMapping(input.uv,viewDir);
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    discard;
	float3 normalFromMap = normalMap.Sample(trillinear, texCoords).rgb;
	normalFromMap = normalFromMap * 2 - 1;	// Normal unpacking
	input.normal = normalFromMap;

	float dist = distance(input.TangentPLightPos,input.TangentFragPos);
	//float dist = distance(input.TangentPLightPos,input.TangentFragPos);
	float3 dirTowardsPointLight = normalize(input.TangentPLightPos - input.TangentFragPos);
	float3 dirTowardsCamera = normalize(input.TangentViewPos - input.TangentFragPos);
	float3 surfaceColor = diffuseTexture.Sample(trillinear, texCoords).rgb;
	
	output =
	pointLight.PointLightColor * CalculatePointLight(input.normal, dirTowardsPointLight, dist)// PointLight
	+ CalculateDirectionalLight(input.normal, directionLight, input.TangentDLightPos);										// DirectionalLight
	return float4(output, 1) *float4(surfaceColor,1) ;//+ float4(SpecLight(normalFromMap, dirTowardsCamera, dirTowardsPointLight, specularLight.SpecularStrength).xxx, 1);
}