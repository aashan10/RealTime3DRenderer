#version 460 core
#extension GL_GOOGLE_include_directive: require

#define FORWARD_FRAGMENT_SHADER
#include "config.h"
#define PI 3.1415926538f

layout(location = COLOR_ATTACHMENT0) out vec4 fragColor;

layout (location = VERT_INTERFACE_BLOCK_LOC) in VertexData {
   vec2 texCoord;
   vec3 worldPos;
   vec3 normal;
   vec3 camPos;
} fs_in;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
    float intensity;  // lumen
};

layout(location = FRAG_U_MATERIAL_ALBEDO_LOC) uniform Material material;
layout(location = FRAG_U_POINT_LIGHT0_POS) uniform PointLight pointLights[MAX_POINT_LIGHTS];
layout(location = FRAG_U_POINT_LIGHT_SIZE) uniform int pointLightSize;

layout(binding = FRAG_U_IRRADIANCE_MAP_BND) uniform samplerCube irradianceMap;
layout(binding = FRAG_U_PREFILTERED_MAP_BND) uniform samplerCube prefilteredMap;
layout(binding = FRAG_U_BRDF_INTEGRATION_MAP_BND) uniform sampler2D brdfIntegrationMap;

float invSqureAttenuation(float distance, float radius) {
    return pow(clamp(1.0f - pow((distance / radius), 4.0f), 0.0f, 1.0f), 2.0f)/(distance * distance + 1.0f);
}

// calculates ratio of light that gets reflected
vec3 fresnelSchlick(float VoH, vec3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - VoH, 5.0f);
}

// fresnelSchlick that takes roughness into account for indirect lighting using irradianceMap
vec3 fresnelSchlickRoughness(float VoH, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - VoH, 5.0f);
}


// NDF for microfactes oriented toward halfway
float distributionGGX(float NoH, float roughness) {
    float r = roughness * roughness;
    float r2 = r * r; // gives better result based on observation by disney and epic games.
    float NoH2 = NoH*NoH;
    float d = (NoH2 * (r2 - 1.0f) + 1.0f);
    return r2 / (d * d * PI);
}

// Geometry Function for microfacets shadowing & masking
float geometrySchlickGGX(float NoV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    return NoV / (NoV * (1.0f - k) + k);
}

float geometrySmith(float NoV, float NoL, float roughness) {
    float ggxL = geometrySchlickGGX(NoL, roughness);
    float ggxV = geometrySchlickGGX(NoV, roughness);
    return ggxL * ggxV;
}

void main() {
    vec3 normal = normalize(fs_in.normal);
    vec3 viewDir = normalize(fs_in.camPos - fs_in.worldPos);
    vec3 R = reflect(-viewDir, normal); // reflect viewDir on normal

    // calculate surface reflection at zero incidence
    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, material.albedo, material.metallic);

    float NoV = max(dot(normal, viewDir), 0.0f);
    /**
     * Direct lighting
     *
     * calculate total reflected irradiance by current fragment using
     * reflectance equation.
     * Where the light sources are our point lights.
     */
    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < pointLightSize; ++i) {
        // fragment radiance per pixel
        vec3 lwSub = pointLights[i].position - fs_in.worldPos;
        vec3 lightDir = normalize(lwSub);
        vec3 halfway = normalize(lightDir + viewDir);
        float distance = length(lwSub);
        float attenuation = invSqureAttenuation(distance, pointLights[i].radius);
        vec3 radiance = pointLights[i].color * pointLights[i].intensity * attenuation;

        // apply cook-torrance brdf (D*F*G)/(4(Wo.n)*(Wi*n)
        float NoL = max(dot(normal, lightDir), 0.0f);

        float roughness = max(material.roughness, 0.05f);
        float NDF = distributionGGX(max(dot(normal, halfway), 0.0f), roughness);
        float G = geometrySmith(NoV, NoL, roughness);
        vec3 F = fresnelSchlick(clamp(dot(viewDir, halfway), 0.0f, 1.0f), F0);

        vec3 num = NDF * G * F;
        float denom = 4.0f * NoV * NoL;
        vec3 specular = num / max(denom, 0.001f); // to prevent denom from being zero

        vec3 kD = vec3(1.0f) - F; // F is specular ratio

        /**
         * Metallic material dont have diffuse.
         * Linear blend if partly metal (pure metals have no diffuse light).
         */
        kD *= 1.0f - material.metallic;

        // add to total outgoing radiance Lo
        Lo += (kD * material.albedo / PI + specular) * radiance * NoL;
    }
    // apply ambient light with IBL (Indirect lighting)
    vec3 kS = fresnelSchlickRoughness(clamp(dot(viewDir, normal), 0.0f, 1.0f), F0, material.roughness);
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - material.metallic;
    vec3 irradiance = texture(irradianceMap, normal).rgb;
    vec3 diffuse = kD * (irradiance * material.albedo);

    const float MAX_REFLECTION_LOD = 4.0f; // max mipLevel for prefilteredMap
    vec3 prefilteredColor = textureLod(prefilteredMap, R, material.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(brdfIntegrationMap, vec2(NoV, material.roughness)).rg;
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    vec3 ambient = (diffuse + specular) * material.ao;
    vec3 color = ambient + Lo;
    // set fragColor
    fragColor = vec4(color, 1.0f);
}
