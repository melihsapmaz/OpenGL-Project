#include "Mesh.h"

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures) {
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	VAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO VBO(vertices);
	// Generates Element Buffer Object and links it to indices
	EBO EBO(indices);
	// Links VBO attributes such as coordinates and colors to VAO
	VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*) 0);
	VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*) (3 * sizeof(float)));
	VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*) (6 * sizeof(float)));
	VAO.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*) (9 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}


void Mesh::Draw(
    Shader& shader,
    Camera& camera,
    glm::mat4 matrix,
    glm::vec3 translation,
    glm::quat rotation,
    glm::vec3 scale
) {

    GLuint currentTextureUnit = 0; // Or manage this more robustly if you have many texture types
    // Bind shader and VAO
    shader.Activate();
    VAO.Bind();

    // Track texture slots
    GLuint diffuseUnit = 0;
    GLuint specularUnit = 2;  // reserve unit 2 for specular (avoid conflict with 0/1)

    bool diffuse0Set = false;
    bool diffuse1Set = false;

    // In Mesh::Draw, modify texture binding:
    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string type = textures[i].type;

        if (type == "diffuse") {
            // First, set the unit property of the texture
            textures[i].unit = 0;  // Explicitly set unit to match what shader expects
            textures[i].texUnit(shader, "diffuse0", 0);
            textures[i].Bind();
        } else if (type == "specular") {
            textures[i].unit = 1;  // Explicitly set unit to match what shader expects
            textures[i].texUnit(shader, "specularMap", 1);
            textures[i].Bind();
        }
    }

    // Pass camera position
    glUniform3f(glGetUniformLocation(shader.ID, "camPos"),
                camera.Position.x, camera.Position.y, camera.Position.z);

    // Set camera and view matrices
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

    // Construct transformation matrices
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 rot = glm::mat4_cast(rotation);
    glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);

    // Send transformation matrices to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "translation"), 1, GL_FALSE, glm::value_ptr(trans));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "rotation"), 1, GL_FALSE, glm::value_ptr(rot));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "scale"), 1, GL_FALSE, glm::value_ptr(sca));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));

    // Set default lighting parameters if they're not already set elsewhere
    glUniform1f(glGetUniformLocation(shader.ID, "shininess"), 32.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "specularStrength"), 0.5f);
    glUniform1f(glGetUniformLocation(shader.ID, "diffuseStrength"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "ambientStrength"), 0.1f);

    // Set light position/properties for dirLight
    glUniform3f(glGetUniformLocation(shader.ID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
    glUniform3f(glGetUniformLocation(shader.ID, "dirLight.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shader.ID, "dirLight.diffuse"), 0.7f, 0.7f, 0.7f);
    glUniform3f(glGetUniformLocation(shader.ID, "dirLight.specular"), 1.0f, 1.0f, 1.0f);

    // Add after setting dirLight properties
    glUniform3f(glGetUniformLocation(shader.ID, "pointLights[0].position"), 0.0f, 5.0f, 0.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "pointLights[0].constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "pointLights[0].linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shader.ID, "pointLights[0].quadratic"), 0.032f);
    glUniform3f(glGetUniformLocation(shader.ID, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
    glUniform3f(glGetUniformLocation(shader.ID, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
    glUniform3f(glGetUniformLocation(shader.ID, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);

    // Set default spotlight params to minimize its effect if not used
    glUniform3f(glGetUniformLocation(shader.ID, "spotLight.position"), 0.0f, 0.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shader.ID, "spotLight.direction"), 0.0f, -1.0f, 0.0f);
    glUniform1f(glGetUniformLocation(shader.ID, "spotLight.cutOff"), 0.0f);  // Very narrow cone
    glUniform1f(glGetUniformLocation(shader.ID, "spotLight.outerCutOff"), 0.0f);

    glUniform1f(glGetUniformLocation(shader.ID, "textureBlendFactor"), 0.0f);  // Use only diffuse0

    // We also need to set the camera position for specular calculations
    glUniform3f(glGetUniformLocation(shader.ID, "viewPos"),
                camera.Position.x, camera.Position.y, camera.Position.z);

    // Draw mesh
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
