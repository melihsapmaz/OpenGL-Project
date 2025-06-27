#include "Texture.h"

Texture::Texture(const char* image, const char* texType, GLuint slot) {

	// Assign texture type (e.g., "diffuse", "specular")
	type = texType;

	// Store texture slot
	unit = slot;

	// Image load
    // Image load
    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

    // Check if image loaded successfully
    bool usingErrorTexture = false;
    if (bytes == NULL) {
        // Create a small error texture (bright pink)
        widthImg = heightImg = 1;
        numColCh = 3;
        unsigned char errorPixel[] = {255, 0, 255}; // Bright pink
        bytes = errorPixel;

        usingErrorTexture = true;
    }

    // Generate texture ID  
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    // Format detection and upload
    if (numColCh == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
    } else if (numColCh == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
    } else if (numColCh == 2) {
        // Handle 2 channel images - interpret as RG format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, widthImg, heightImg, 0, GL_RG, GL_UNSIGNED_BYTE, bytes);
    } else if (numColCh == 1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, widthImg, heightImg, 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
    } else {
        std::cerr << "Unusual number of color channels (" << numColCh << ") in texture: " << image << std::endl;

        // Create a fallback texture (yellow)
        unsigned char fallbackPixel[] = {255, 255, 0};  // Yellow
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, fallbackPixel);
    }

    // Generate mipmaps and cleanup
    glGenerateMipmap(GL_TEXTURE_2D);
    if (!usingErrorTexture && bytes != NULL) {
        stbi_image_free(bytes);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint textureUnitToSampleFrom) {
	GLuint texLoc = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(texLoc, textureUnitToSampleFrom); // Tell sampler which unit to use
}

void Texture::Bind() {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete() {
	glDeleteTextures(1, &ID);
}
