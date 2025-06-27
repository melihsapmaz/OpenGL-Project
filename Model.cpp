#include"Model.h"

Model::Model(const char* file) {
	// Make a JSON object
	std::string text = get_file_contents(file);
	JSON = json::parse(text);

	// Get the binary data
	Model::file = file;
	data = getData();

	// Traverse all nodes
	traverseNode(0);
}

void Model::Draw(Shader& shader, Camera& camera) {
	if (meshes.empty()) {
		std::cout << "WARNING: Model has no meshes to draw." << std::endl;
		return;
	}

	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Mesh::Draw(shader, camera, matricesMeshes[i]);
	}
}

void Model::Draw(Shader& shader, Camera& camera, glm::mat4 modelMatrix) {
	if (meshes.empty()) {
		std::cout << "WARNING: Model has no meshes to draw." << std::endl;
		return;
	}
	// Go over all meshes and draw each one
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Mesh::Draw(shader, camera, modelMatrix * matricesMeshes[i]);
	}
}


void Model::loadMesh(unsigned int indMesh) {
	// Get all accessor indices
	auto& prim = JSON["meshes"][indMesh]["primitives"][0]["attributes"];
	auto& primRoot = JSON["meshes"][indMesh]["primitives"][0];

	if (!prim.contains("POSITION"))
		std::cerr << "  Missing POSITION attribute!" << std::endl;
	if (!primRoot.contains("indices"))
		std::cerr << "  Missing indices attribute!" << std::endl;

	// Get material for this mesh
	unsigned int materialIndex = 0;
	if (primRoot.contains("material")) { // Check primitives[0] for material, not meshes
		materialIndex = primRoot["material"];
	}

	// Get vertex positions, normals, UVs and indices
	unsigned int posAccInd = prim["POSITION"];
	unsigned int normalAccInd = prim["NORMAL"];
	unsigned int texAccInd = prim["TEXCOORD_0"];
	unsigned int indAccInd = primRoot["indices"];

	// Get vertices data
	std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
	std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
	std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
	std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
	std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
	std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);

	// Check for size mismatches
	if (positions.size() != normals.size() || positions.size() != texUVs.size()) {
		std::cerr << "WARNING: Mismatched attribute counts!" << std::endl;
	}

	// Combine all the vertex components
	std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
	std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);

	// Get textures for this mesh using its material
	std::vector<Texture> textures = getTextures(materialIndex);

	// Create mesh and add to list
	meshes.push_back(Mesh(vertices, indices, textures));

	std::cout << "Mesh " << indMesh << " uses material " << materialIndex << std::endl;


}


void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix) {
	// Current node
	json node = JSON["nodes"][nextNode];

	// Declare translation at the start
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);

	// Get translation if it exists
	
	// Get translation if it exists
	if (node.contains("translation") && node["translation"].is_array()) {
		float transValues[3] = {0.0f, 0.0f, 0.0f};
		for (unsigned int i = 0; i < node["translation"].size() && i < 3; i++)
			if (node["translation"][i].is_number()) transValues[i] = node["translation"][i];
		translation = glm::make_vec3(transValues);
	}


	// Get quaternion if it exists
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end()) {
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}
	// Get scale if it exists
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end()) {
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}
	// Get matrix if it exists
	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end()) {
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	// Initialize matrices
	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	// Use translation, rotation, and scale to change the initialized matrices
	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	// Multiply all matrices together
	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	// Check if the node contains a mesh and if it does load it
	if (node.find("mesh") != node.end()) {
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	// Check if the node has children, and if it does, apply this function to them with the matNextNode
	if (node.find("children") != node.end()) {
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData() {
	// Create a place to store the raw text, and get the uri of the .bin file
	std::string bytesText;
	std::string uri = JSON["buffers"][0]["uri"];

	// Store raw text data into bytesText
	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
	bytesText = get_file_contents((fileDirectory + uri).c_str());

	// Transform the raw text data into bytes and put them in a vector
	std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
	return data;
}

std::vector<float> Model::getFloats(json accessor) {
	std::vector<float> floatVec;

	// Add to getFloats() at the beginning:
	if (!accessor.contains("componentType") || accessor["componentType"] != 5126) {
		std::cerr << "WARNING: Expected float component type (5126) for accessor" << std::endl;
		if (accessor.contains("componentType")) {
			std::cerr << "Found component type: " << accessor["componentType"] << std::endl;
		}
	}


	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 1);
	unsigned int count = accessor.contains("count") && accessor["count"].is_number_unsigned() ? accessor["count"].get<unsigned int>() : 0;
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	std::string type = accessor.contains("type") && accessor["type"].is_string() ? accessor["type"] : "SCALAR";

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView.value("byteOffset", 0);

	// Get stride, defaulting to tightly packed if not specified
	unsigned int stride = 0;
	if (bufferView.contains("byteStride") && bufferView["byteStride"].is_number_unsigned()) {
		stride = bufferView["byteStride"].get<unsigned int>();
	}

	// Interpret the type and store it into numPerVert
	unsigned int numPerVert;
	if (type == "SCALAR") numPerVert = 1;
	else if (type == "VEC2") numPerVert = 2;
	else if (type == "VEC3") numPerVert = 3;
	else if (type == "VEC4") numPerVert = 4;
	else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

	// If stride is 0, data is tightly packed - use the size of the vertex component
	if (stride == 0) {
		stride = numPerVert * 4; // Each component is 4 bytes (float)
	}

	// Go over all the data using stride for proper spacing
	unsigned int beginningOfData = byteOffset + accByteOffset;
	for (unsigned int i = 0; i < count; i++) {
		unsigned int dataOffset = beginningOfData + (i * stride);

		// Extract each component of the vertex
		for (unsigned int j = 0; j < numPerVert; j++) {
			unsigned int bytePosition = dataOffset + (j * 4);

			// Make sure we don't go past the end of data array
			if (bytePosition + 3 >= data.size()) {
				std::cerr << "Warning: Attempting to read beyond data bounds at position "
					<< bytePosition << " (data size: " << data.size() << ")" << std::endl;
				continue;
			}

			unsigned char bytes[] = {data[bytePosition], data[bytePosition + 1],
									data[bytePosition + 2], data[bytePosition + 3]};
			float value;
			std::memcpy(&value, bytes, sizeof(float));
			floatVec.push_back(value);
		}
	}

	return floatVec;
}


std::vector<GLuint> Model::getIndices(json accessor) {
	std::vector<GLuint> indices;

	// Get properties from the accessor
	unsigned int buffViewInd = accessor.value("bufferView", 0);
	unsigned int count = accessor.contains("count") && accessor["count"].is_number_unsigned() ? accessor["count"].get<unsigned int>() : 0;
	unsigned int accByteOffset = accessor.value("byteOffset", 0);
	unsigned int componentType = accessor.contains("componentType") && accessor["componentType"].is_number_unsigned() ? accessor["componentType"].get<unsigned int>() : 5123;

	// Get properties from the bufferView
	json bufferView = JSON["bufferViews"][buffViewInd];
	unsigned int byteOffset = bufferView.value("byteOffset", 0); // Use value() in case byteOffset is missing

	// Get byte length of the buffer view
	unsigned int byteLength = bufferView.value("byteLength", 0);

	// Calculate component size
	unsigned int componentSize = 0;
	if (componentType == 5125) componentSize = 4;      // uint32
	else if (componentType == 5123) componentSize = 2; // uint16
	else if (componentType == 5122) componentSize = 2; // int16

	// Get stride, defaulting to component size if not specified
	unsigned int stride = 0;
	if (bufferView.contains("byteStride") && bufferView["byteStride"].is_number_unsigned()) {
		stride = bufferView["byteStride"].get<unsigned int>();
	}

	// If stride is 0, use the component size (tightly packed data)
	if (stride == 0) {
		stride = componentSize;
	}

	// Extra safety checks
	unsigned int beginningOfData = byteOffset + accByteOffset;
	unsigned int dataEndOffset = beginningOfData + (count * stride);

	if (dataEndOffset > byteOffset + byteLength) {
		std::cerr << "WARNING: Calculated data end exceeds buffer view length!" << std::endl;
		std::cerr << "  - Data end offset: " << dataEndOffset << std::endl;
		std::cerr << "  - Buffer view end: " << (byteOffset + byteLength) << std::endl;
	}

	if (beginningOfData >= data.size() || dataEndOffset > data.size()) {
		std::cerr << "ERROR: Accessor data would exceed buffer size!" << std::endl;
		return indices; // Return empty indices to avoid crash
	}

	// Loop through the indices
	for (unsigned int i = 0; i < count; i++) {
		unsigned int dataOffset = beginningOfData + (i * stride);

		// Bounds checking
		if (dataOffset + componentSize > data.size()) {
			std::cerr << "Error: Index " << i << " offset " << dataOffset << " exceeds buffer size " << data.size() << std::endl;
			continue;
		}

		// Extract based on component type
		if (componentType == 5125) {  // unsigned int (4 bytes)
			unsigned char bytes[] = {data[dataOffset], data[dataOffset + 1], data[dataOffset + 2], data[dataOffset + 3]};
			unsigned int value;
			std::memcpy(&value, bytes, sizeof(unsigned int));
			indices.push_back((GLuint) value);
		} else if (componentType == 5123) {  // unsigned short (2 bytes)
			unsigned char bytes[] = {data[dataOffset], data[dataOffset + 1]};
			unsigned short value;
			std::memcpy(&value, bytes, sizeof(unsigned short));
			indices.push_back((GLuint) value);
		} else if (componentType == 5122) {  // short (2 bytes)
			unsigned char bytes[] = {data[dataOffset], data[dataOffset + 1]};
			short value;
			std::memcpy(&value, bytes, sizeof(short));
			indices.push_back((GLuint) value);
		}
	}

	return indices;
}

std::vector<Texture> Model::getTextures(unsigned int materialIndex) {
	std::vector<Texture> textures;
	bool hasSpecular = false;

	std::string fileStr = std::string(file);
	std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

	// Check if material exists
	if (!JSON.contains("materials") || materialIndex >= JSON["materials"].size()) {
		std::cerr << "Warning: Material " << materialIndex << " not found in GLTF" << std::endl;
		return textures;
	}

	// Get material
	auto& material = JSON["materials"][materialIndex];

	// Check for PBR data
	if (material.contains("pbrMetallicRoughness")) {
		auto& pbr = material["pbrMetallicRoughness"];

		// Base color texture
		if (pbr.contains("baseColorTexture") && pbr["baseColorTexture"].contains("index")) {
			int textureIndex = pbr["baseColorTexture"]["index"];
			int sourceIndex = JSON["textures"][textureIndex]["source"];
			std::string texPath = JSON["images"][sourceIndex]["uri"];
			std::cout << "Material " << materialIndex << " loading texture: " << (fileDirectory + texPath) << std::endl;


			// Check if already loaded
			bool skip = false;
			for (unsigned int j = 0; j < loadedTexName.size(); j++) {
				if (loadedTexName[j] == fileDirectory + texPath) {
					textures.push_back(loadedTex[j]);
					skip = true;
					break;
				}
			}

			if (!skip) {
				Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
				textures.push_back(diffuse);
				loadedTex.push_back(diffuse);
				loadedTexName.push_back(fileDirectory + texPath);
			}
		}

		// Metallic-Roughness texture
		if (pbr.contains("metallicRoughnessTexture") && pbr["metallicRoughnessTexture"].contains("index")) {
			int textureIndex = pbr["metallicRoughnessTexture"]["index"];
			int sourceIndex = JSON["textures"][textureIndex]["source"];
			std::string texPath = JSON["images"][sourceIndex]["uri"];

			// Check if already loaded
			bool skip = false;
			for (unsigned int j = 0; j < loadedTexName.size(); j++) {
				if (loadedTexName[j] == fileDirectory + texPath) {
					textures.push_back(loadedTex[j]);
					
					hasSpecular = true;
					skip = true;
					break;
				}
			}

			if (!skip) {
				std::cout << "Loading metallic-roughness texture for material " << materialIndex << ": " << texPath << std::endl;
				Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
				textures.push_back(specular);
				loadedTex.push_back(specular);
				loadedTexName.push_back(fileDirectory + texPath);
				hasSpecular = true;
			}
		}
	}

	// If no specular map was loaded, create a default one directly
	if (!hasSpecular) {
		GLuint whiteTexID;
		glGenTextures(1, &whiteTexID);
		glBindTexture(GL_TEXTURE_2D, whiteTexID);

		// Create a single white pixel
		unsigned char whitePixel[3] = {255, 255, 255};
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Create texture object with in-memory data
		std::string defaultSpecName = "default_specular_" + std::to_string(materialIndex);
		Texture defaultSpec(defaultSpecName.c_str(), "specular", loadedTex.size());
		defaultSpec.ID = whiteTexID; // Directly set ID without file loading

		textures.push_back(defaultSpec);
		loadedTex.push_back(defaultSpec);
		loadedTexName.push_back(defaultSpecName);
	}


	return textures;
}



std::vector<Vertex> Model::assembleVertices(
	std::vector<glm::vec3> positions,
	std::vector<glm::vec3> normals,
	std::vector<glm::vec2> texUVs
) {
	std::vector<Vertex> vertices;

	// Get the minimum size to avoid out-of-bounds access
	size_t vertexCount = std::min({positions.size(), normals.size(), texUVs.size()});

	vertices.reserve(vertexCount);
	for (size_t i = 0; i < vertexCount; i++) {
		vertices.push_back(
			Vertex{
				positions[i],
				normals[i],
				glm::vec3(1.0f, 1.0f, 1.0f),  // This is white
				texUVs[i]
			}
		);

	}

	return vertices;
}


std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 2;

	std::vector<glm::vec2> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec2(0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 3;

	std::vector<glm::vec3> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec3(0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec) {
	const unsigned int floatsPerVector = 4;

	std::vector<glm::vec4> vectors;
	for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector) {
		vectors.push_back(glm::vec4(0, 0, 0, 0));

		for (unsigned int j = 0; j < floatsPerVector; j++) {
			vectors.back()[j] = floatVec[i + j];
		}
	}
	return vectors;
}

// In your model constructor or immediately after loading:
void Model::CalculateBoundingBox() {
	minBounds = glm::vec3(FLT_MAX);
	maxBounds = glm::vec3(-FLT_MAX);

	// Loop through all meshes to find overall bounds
	if (meshes.empty()) {
		std::cout << "WARNING: No meshes in model, cannot calculate bounding box!" << std::endl;
		// Set some default bounds
		minBounds = glm::vec3(-1.0f);
		maxBounds = glm::vec3(1.0f);
		return;
	}

	for (const Mesh& mesh : meshes) {
		for (const Vertex& vertex : mesh.vertices) {
			minBounds.x = std::min(minBounds.x, vertex.position.x);
			minBounds.y = std::min(minBounds.y, vertex.position.y);
			minBounds.z = std::min(minBounds.z, vertex.position.z);

			maxBounds.x = std::max(maxBounds.x, vertex.position.x);
			maxBounds.y = std::max(maxBounds.y, vertex.position.y);
			maxBounds.z = std::max(maxBounds.z, vertex.position.z);
		}
	}

	std::cout << "Model bounds: min(" << minBounds.x << "," << minBounds.y << ","
		<< minBounds.z << ") max(" << maxBounds.x << "," << maxBounds.y << ","
		<< maxBounds.z << ")" << std::endl;
}


/// In Model.cpp
bool Model::RayIntersectsModel(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
							   float maxDistance, const glm::mat4& modelMatrix) const {
	// Transform the ray into model local space
	glm::mat4 invModel = glm::inverse(modelMatrix);
	glm::vec3 localOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
	glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDirection, 0.0f)));

	float hitDistance;
	if (RayIntersectsAABB(localOrigin, localDir, minBounds, maxBounds, hitDistance)) {
		glm::vec3 hitPointLocal = localOrigin + localDir * hitDistance;
		glm::vec3 hitPointWorld = glm::vec3(modelMatrix * glm::vec4(hitPointLocal, 1.0f));
		float worldDistance = glm::distance(rayOrigin, hitPointWorld);
		std::cout << "Model hit: " << file << " at distance " << worldDistance << std::endl;
		if (worldDistance <= maxDistance) {
			return true;
		}
	}

	return false;
}



bool Model::RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
							  const glm::vec3& boxMin, const glm::vec3& boxMax, float& distance) const {
	glm::vec3 invDir = 1.0f / rayDirection;

	glm::vec3 tMin = (boxMin - rayOrigin) * invDir;
	glm::vec3 tMax = (boxMax - rayOrigin) * invDir;
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

	// Only treat as collision if intersection is in front of the ray origin
	if (tNear > tFar || tFar < 0.0f) {
		return false; // No intersection
	}
	if (tNear < 0.0f) {
		// Intersection is behind the ray origin (origin is inside or past the box)
		return false;
	}
	distance = tNear;
	return true;
}

