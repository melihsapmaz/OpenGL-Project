#include "Model.h" // Assumes Model.h includes necessary headers like Camera.h, Shader.h, glad, glfw, glm, stb_image, etc.

// Window dimensions
const unsigned int width = 1366;
const unsigned int height = 768;

// It's good practice to declare stb_image implementation in one .cpp file (e.g., main.cpp or a dedicated .cpp)
// If it's not already in Model.cpp or another included file.
// #define STB_IMAGE_IMPLEMENTATION // Only if not defined elsewhere
// #include "stb_image.h" // Make sure this is included if Texture.cpp or Model.cpp doesn't handle it globally

void checkGLError(const char* operation) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error " << err << " during " << operation << std::endl;
    }
}

int main() {
    // At the top of your file, after the includes
    float lastFrame = 0.0f; // Time of last frame
    float rotationAngle = 0.0f; // Current rotation angle

    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Project - Imported Model", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, width, height);

    // Load Shader
    Shader shaderProgram("default.vert", "default.frag"); // Ensure these shader files exist and are correct

    Camera camera(width, height, glm::vec3(7.0f, 1.7f, 7.0f)); // Start in an open area
    camera.Position = glm::vec3(7.0f, 1.7f, 7.0f); // Typical human eye height
    camera.Orientation = glm::vec3(-1.0f, 0.0f, -1.0f); // Looking toward the scene
    
    // First, adjust the transformation matrices to position the models properly
    glm::mat4 buildingModelMatrix = glm::mat4(1.0f);
	// = glm::scale(buildingModelMatrix, glm::vec3(10.0f)); // Scale the building model

    glm::mat4 dogModelMatrix = glm::mat4(1.0f);
    dogModelMatrix = glm::translate(dogModelMatrix, glm::vec3(3.0f, 0.0f, 2.0f));
    dogModelMatrix = glm::rotate(dogModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    dogModelMatrix = glm::scale(dogModelMatrix, glm::vec3(0.5f));

    glm::mat4 dogModelMatrix2 = glm::mat4(1.0f);
    dogModelMatrix2 = glm::translate(dogModelMatrix2, glm::vec3(4.0f, 0.0f, 1.0f));
    dogModelMatrix2 = glm::rotate(dogModelMatrix2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    dogModelMatrix2 = glm::scale(dogModelMatrix2, glm::vec3(1.3f));
    
    glm::mat4 dogModelMatrix3 = glm::mat4(1.0f);
    dogModelMatrix3 = glm::translate(dogModelMatrix3, glm::vec3(1.0f, 0.0f, 3.0f));
    dogModelMatrix3 = glm::rotate(dogModelMatrix3, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    dogModelMatrix3 = glm::scale(dogModelMatrix3, glm::vec3(2.0f));

    glm::mat4 dogModelMatrix4 = glm::mat4(1.0f);
    dogModelMatrix4 = glm::translate(dogModelMatrix4, glm::vec3(0.0f, 0.0f, 3.0f));
    dogModelMatrix4 = glm::rotate(dogModelMatrix4, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    

    glm::mat4 maleHumanMatrix = glm::mat4(1.0f);
    maleHumanMatrix = glm::translate(maleHumanMatrix, glm::vec3(2.5f, 0.0f, 0.0f));
    //maleHumanMatrix = glm::rotate(maleHumanMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //maleHumanMatrix = glm::rotate(maleHumanMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 femaleHumanMatrix = glm::mat4(1.0f);
    femaleHumanMatrix = glm::translate(femaleHumanMatrix, glm::vec3(1.0f, 0.0f, -2.0f));

    // Load models
	Model model_building("models/building/scene.gltf"); // building model
	Model model_female_human("models/female_human/scene.gltf"); // female human model
	Model model_male_human("models/male_human/scene.gltf"); // male human model
	Model model_dog("models/dog/scene.gltf"); // dog model

    // Register models for collision detection
    // After loading models in Main.cpp:
    model_building.CalculateBoundingBox();
    model_female_human.CalculateBoundingBox();
    model_male_human.CalculateBoundingBox();
    model_dog.CalculateBoundingBox();

    // Register model instances with their transforms
    camera.AddModelInstance(&model_building, buildingModelMatrix);
    camera.AddModelInstance(&model_female_human, femaleHumanMatrix);
    camera.AddModelInstance(&model_male_human, maleHumanMatrix);
    camera.AddModelInstance(&model_dog, dogModelMatrix);
    camera.AddModelInstance(&model_dog, dogModelMatrix2);
    camera.AddModelInstance(&model_dog, dogModelMatrix3);
    camera.AddModelInstance(&model_dog, dogModelMatrix4);
    
    // Then register the models for collision (as you already do)
    camera.AddCollidableModel(&model_building);
    camera.AddCollidableModel(&model_female_human);
    camera.AddCollidableModel(&model_male_human);
    camera.AddCollidableModel(&model_dog);

    // Light settings
    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // White light
    //glm::vec3 dirLightDir = glm::normalize(glm::vec3(0.5f, -1.0f, -0.5f)); // Adjusted for better visibility
    glm::vec3 dirLightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)); // Straight down light
    glm::vec3 pointLightPos = glm::vec3(0.0f, 2.0f, 2.0f); // Adjusted point light position
    // spotLightPos and spotLightDir will be updated in the loop based on camera

    std::vector<glm::vec3> roomLightPositions = {
        glm::vec3(0.0f, 2.5f, 0.0f),   // Center room
        glm::vec3(5.0f, 2.5f, 0.0f),   // Right room
        glm::vec3(-5.0f, 2.5f, 0.0f),  // Left room
        glm::vec3(0.0f, 2.5f, 5.0f),   // Back room
        glm::vec3(0.0f, 2.5f, -5.0f)   // Front room
    };

	glfwSwapInterval(0); // Disable vsync for maximum FPS (optional, can be set to 1 for vsync)
    unsigned int fps = 0; // Frame counter for FPS calculation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CW); // or GL_CCW, depending on your model
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime(); // Get current time
        float deltaTime = currentFrame - lastFrame; // Calculate time since last frame
        lastFrame = currentFrame;
		

        float rotationSpeed = 0.0005f; // Adjust this value for faster/slower rotation
        rotationAngle += rotationSpeed * deltaTime;

        if (rotationAngle > 2.0f * glm::pi<float>()) {
            rotationAngle -= 2.0f * glm::pi<float>();
        }
        /*
        static float testTimer = 0.0f;
        testTimer += deltaTime;
        if (testTimer > 5.0f) { // Test every 5 seconds
            testTimer = 0.0f;
            
        }
        */
        // Inside the main loop, add occasional tests :
        //camera.TestDirectCollision(camera.Position);

        // Inside your rendering loop, before drawing the models:

        // Update the third dog's transformation matrix for animation
        // Position the third dog
        // Apply continuous rotation around the Y axis
        dogModelMatrix3 = glm::rotate(dogModelMatrix3, rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));


        // Input
        camera.Inputs(window); // Handles keyboard and mouse input for camera

        // Update camera matrix
        camera.updateMatrix(camera.fov, 0.1f, 100.0f);



        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader program
        shaderProgram.Activate();
        checkGLError("shader activation");

        // Instead of individual uniforms:
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.shininess"), 32.0f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.specularStrength"), 0.5f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.diffuseStrength"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "material.textureBlendFactor"), 0.0f);

        // And for the texture samplers:
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "material.diffuse0"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "material.diffuse1"), 1);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "material.specularMap"), 2);

		checkGLError("set shader uniforms");


        // Set Light uniforms
        glUniform4fv(glGetUniformLocation(shaderProgram.ID, "lightColor"), 1, glm::value_ptr(lightColor));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "viewPos"), 1, glm::value_ptr(camera.Position)); // Shader might use viewPos or camPos
		checkGLError("set light uniforms");

        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "dirLight.ambient"), 1, glm::value_ptr(glm::vec3(0.3f)));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "dirLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f)));


        // Directional light
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "dirLight.direction"), 1, glm::value_ptr(dirLightDir));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "dirLight.specular"), 1, glm::value_ptr(glm::vec3(0.5f))); // Moderate specular
		checkGLError("set directional light uniforms");

        // Set up multiple point lights for interior
        for (int i = 0; i < std::min(3, (int) roomLightPositions.size()); i++) { // Support at least 3 lights
            std::string uniformName = "pointLights[" + std::to_string(i) + "].";

            glUniform3fv(glGetUniformLocation(shaderProgram.ID, (uniformName + "position").c_str()),
                         1, glm::value_ptr(roomLightPositions[i]));
            glUniform3fv(glGetUniformLocation(shaderProgram.ID, (uniformName + "ambient").c_str()),
                         1, glm::value_ptr(glm::vec3(0.05f)));
            glUniform3fv(glGetUniformLocation(shaderProgram.ID, (uniformName + "diffuse").c_str()),
                         1, glm::value_ptr(glm::vec3(0.8f)));
            glUniform3fv(glGetUniformLocation(shaderProgram.ID, (uniformName + "specular").c_str()),
                         1, glm::value_ptr(glm::vec3(1.0f)));
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformName + "constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformName + "linear").c_str()), 0.09f);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, (uniformName + "quadratic").c_str()), 0.032f);
        }
		checkGLError("set point light uniforms");

        // SpotLight (camera-based)
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "spotLight.position"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "spotLight.direction"), 1, glm::value_ptr(camera.Orientation));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "spotLight.ambient"), 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "spotLight.diffuse"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "spotLight.specular"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f))); // Slightly wider outer
		checkGLError("set spot light uniforms");

        model_building.Draw(shaderProgram, camera, buildingModelMatrix);// Building uses identity matrix
        model_dog.Draw(shaderProgram, camera, dogModelMatrix); // Pass dog's matrix
		model_dog.Draw(shaderProgram, camera, dogModelMatrix2); // Pass second dog's matrix
		model_dog.Draw(shaderProgram, camera, dogModelMatrix3); // Pass third dog's matrix
		model_dog.Draw(shaderProgram, camera, dogModelMatrix4); // Pass fourth dog's matrix

        model_female_human.Draw(shaderProgram, camera, femaleHumanMatrix); // Pass female's matrix  
        model_male_human.Draw(shaderProgram, camera, maleHumanMatrix); // Pass male's matrix

        checkGLError("draw call");

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL error: " << err << std::endl;
        }

		nbFrames++;
		// Calculate and print FPS every second
		if (currentFrame - lastTime >= 1.0) { // If a second has passed
			char title[256];
			snprintf(title, sizeof(title), "OpenGL Project - Imported Model - FPS: %d", nbFrames);
			glfwSetWindowTitle(window, title);
			nbFrames = 0; // Reset frame count
			lastTime += 1.0; // Increment last time by 1 second
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    // Model's resources (VAOs, VBOs, Textures) should be deleted
    shaderProgram.Delete(); // Shader class should have a destructor or Delete method
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}