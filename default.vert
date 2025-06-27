#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTex;

out vec3 FragPos_WorldSpace;
out vec3 Normal_WorldSpace;
out vec3 VertexColor;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate fragment position in world space (for lighting calculations)
    FragPos_WorldSpace = vec3(model * vec4(aPos, 1.0));
    
    // Calculate normal in world space
    // Using transpose(inverse(model)) handles non-uniform scaling properly
    Normal_WorldSpace = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass color and texture coordinates to fragment shader
    VertexColor = aColor;
    TexCoords = aTex;
    
    // Calculate final position in clip space
    gl_Position = projection * view * vec4(FragPos_WorldSpace, 1.0);
}



