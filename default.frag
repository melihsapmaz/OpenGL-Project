#version 330 core

out vec4 FragColor;

in vec3 FragPos_WorldSpace; 
in vec3 Normal_WorldSpace;  
in vec3 VertexColor;        
in vec2 TexCoords;          

// Define Material struct
struct Material {
    sampler2D diffuse0;     // Primary diffuse texture
    sampler2D diffuse1;     // Secondary diffuse texture
    sampler2D specularMap;  // Specular map
    float shininess;        // Shininess exponent
    float specularStrength; // Specular intensity multiplier
    float diffuseStrength;  // Diffuse intensity multiplier 
    float ambientStrength;  // Ambient intensity multiplier
    float textureBlendFactor; // Blending between diffuse0 and diffuse1
};
uniform Material material;

// --- Camera Uniforms ---
uniform vec3 viewPos; // Camera position in world space
uniform mat4 view; // View matrix
uniform mat4 projection; // Projection matrix
// --- Normal Calculation ---
vec3 norm = normalize(Normal_WorldSpace); // Normal in world space
vec3 viewDir = normalize(viewPos - FragPos_WorldSpace); // Direction from fragment to camera
// --- Texture Samplers ---
uniform sampler2D diffuse0; // Primary diffuse texture
uniform sampler2D diffuse1; // Secondary diffuse texture


// --- Light Structs ---
struct DirLight {
    vec3 direction;   // Direction from the light source
    vec3 ambient;     // Ambient color/intensity of the light
    vec3 diffuse;     // Diffuse color/intensity of the light
    vec3 specular;    // Specular color/intensity of the light
};
uniform DirLight dirLight;

struct PointLight {
    vec3 position;    // Position in world space
    float constant;   // Attenuation factor
    float linear;     // Attenuation factor
    float quadratic;  // Attenuation factor
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLights[1]; // Array for multiple point lights, using 1 for this example

struct SpotLight {
    vec3 position;
    vec3 direction;   // Direction the spotlight is pointing
    float cutOff;     // Cosine of the inner cone angle
    float outerCutOff;// Cosine of the outer cone angle
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

vec3 calculateLight(vec3 lightDirection_normalized,
                    vec3 lightAmbientColor, vec3 lightDiffuseColor, vec3 lightSpecularColor,
                    float attenuation, float spotIntensityFactor)
{
    // Texture sampling
    vec3 tex0Color = texture(material.diffuse0, TexCoords).rgb;
    vec3 blendedDiffuseColor = tex0Color;
    
    if (material.textureBlendFactor > 0.001 && material.textureBlendFactor < 0.999) {
        blendedDiffuseColor = mix(tex0Color, texture(material.diffuse1, TexCoords).rgb, material.textureBlendFactor);
    } else if (material.textureBlendFactor >= 0.999) {
        blendedDiffuseColor = texture(material.diffuse1, TexCoords).rgb;
    }

    float specularMapValue = texture(material.specularMap, TexCoords).r;

    // Normalize inputs
    vec3 norm = normalize(Normal_WorldSpace);
    vec3 viewDir = normalize(viewPos - FragPos_WorldSpace);

    // Ambient
    vec3 ambient = lightAmbientColor * material.ambientStrength * blendedDiffuseColor;

    // Diffuse
    float diff = max(dot(norm, lightDirection_normalized), 0.0);
    vec3 diffuse = lightDiffuseColor * material.diffuseStrength * diff * blendedDiffuseColor;

    // Specular
    vec3 reflectDir = reflect(-lightDirection_normalized, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightSpecularColor * material.specularStrength * spec * specularMapValue;
    
    return ambient + attenuation * spotIntensityFactor * (diffuse + specular);
}



void main()
{
    //ambientStrength = max(ambientStrength, 0.4);
    vec3 totalLighting = vec3(0.0);

    // --- Directional Light ---
    // Directional light's direction is usually "direction light is coming from"
    // So, the direction to the light source is the negative of that.
    vec3 dirLight_toLightSource = normalize(-dirLight.direction);
    totalLighting += calculateLight(dirLight_toLightSource,
                                   dirLight.ambient, dirLight.diffuse, dirLight.specular,
                                   1.0, // No distance attenuation for directional light
                                   1.0); // No spotlight cone effect

    // --- Point Light (Example for one point light) ---
    if (pointLights.length() > 0) { // Check if array is not empty (though GLSL might not optimize this well)
        vec3 pointLight_toLightSource = normalize(pointLights[0].position - FragPos_WorldSpace);
        float pointDist = length(pointLights[0].position - FragPos_WorldSpace);
        float pointAttenuation = 1.0 / (pointLights[0].constant + pointLights[0].linear * pointDist + pointLights[0].quadratic * (pointDist * pointDist));
        totalLighting += calculateLight(pointLight_toLightSource,
                                       pointLights[0].ambient, pointLights[0].diffuse, pointLights[0].specular,
                                       pointAttenuation,
                                       1.0);
    }

    // --- Spotlight ---
    vec3 spotLight_FragToLightSourceDir = spotLight.position - FragPos_WorldSpace; // Vector from fragment to light position
    float spotDist = length(spotLight_FragToLightSourceDir);
    vec3 spotLight_toLightSource_normalized = normalize(spotLight_FragToLightSourceDir);

    float spotAttenuation = 1.0 / (spotLight.constant + spotLight.linear * spotDist + spotLight.quadratic * (spotDist * spotDist));
    
    // Calculate spotlight intensity based on cone
    // spotLight.direction is the direction the cone is pointing.
    // We need the angle between vector from light to fragment and spotlight's direction.
    vec3 lightToFragDir_normalized = normalize(FragPos_WorldSpace - spotLight.position);
    float theta = dot(lightToFragDir_normalized, normalize(spotLight.direction));
    
    float spotEffect = 0.0;
    // spotLight.cutOff is cos(innerAngle), spotLight.outerCutOff is cos(outerAngle)
    // Cosine is larger for smaller angles. So innerCutOff (cos(inner)) > outerCutOff (cos(outer)).
    if (theta > spotLight.outerCutOff) { // Check if fragment is within the outer cone
        float epsilon = spotLight.cutOff - spotLight.outerCutOff;
        if (epsilon <= 0.0) epsilon = 0.001; // Prevent division by zero or negative if cutoffs are misconfigured
        spotEffect = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
    }
    
    totalLighting += calculateLight(spotLight_toLightSource_normalized, // Direction from fragment to light
                                   spotLight.ambient, spotLight.diffuse, spotLight.specular,
                                   spotAttenuation,
                                   spotEffect);

    // Final output with a debug switch
    bool debugMode = false;
    if (debugMode) {
        // Debug options:
        // Show normals
        //FragColor = vec4(normalize(Normal_WorldSpace) * 0.5 + 0.5, 1.0);
        // Show texture coordinates
        //FragColor = vec4(TexCoords, 0.0, 1.0);
        // Show only texture
        FragColor = texture(diffuse0, TexCoords);
    } else {
        // Regular lighting calculation
        FragColor = vec4(totalLighting, 1.0);
    }
    
    //FragColor = vec4(totalLighting, 1.0);

    // Debugging light contributions
    //FragColor = vec4(1.0f,0.0f,0.0f,1.0f); // Final color output


    // If you want to include vertex color:
    // FragColor = vec4(totalLighting * VertexColor, 1.0); // Or some other blending
}

