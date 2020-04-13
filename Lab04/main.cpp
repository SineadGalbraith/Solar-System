// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Include GLM functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

// Loading photos
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "./assets/models/sun.dae"
#define MESH_NAME2 "./assets/models/mercury.dae"
#define MESH_NAME3 "./assets/models/venus.dae"
#define MESH_NAME4 "./assets/models/earth.dae"
#define MESH_NAME5 "./assets/models/moon.dae"
#define MESH_NAME6 "./assets/models/mars.dae"
#define MESH_NAME7 "./assets/models/jupiter.dae"
#define MESH_NAME8 "./assets/models/saturn.dae"
#define MESH_NAME9 "./assets/models/saturn_ring.dae"
#define MESH_NAME10 "./assets/models/uranus.dae"
#define MESH_NAME11 "./assets/models/neptune.dae"
#define MESH_NAME12 "./assets/models/pluto.dae"
#define MESH_NAME13 "./assets/models/spaceship.dae"

// Textures
const char *sun = "./assets/textures/sun_texture.jpg";
const char *mercury = "./assets/textures/mercury_texture.jpg";
const char *venus = "./assets/textures/venus_texture.jpg";
const char *earth = "./assets/textures/earth_texture.jpg";
const char *moon = "./assets/textures/moon_texture.jpg";
const char *mars = "./assets/textures/mars_texture.jpg";
const char *jupiter = "./assets/textures/jupiter_texture.jpg";
const char *saturn = "./assets/textures/saturn_texture.jpg";
const char *uranus = "./assets/textures/uranus_texture.jpg";
const char *neptune = "./assets/textures/neptune_texture.jpg";
const char *pluto = "./assets/textures/pluto_texture.jpg";

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint sunShader, planetShader;
GLuint cubemapTexture;
glm::vec3 camera(0.0f, 0.0f, 30.f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

vector<std::string> faces
{
	"cwd_rt.jpg",
	"cwd_lf.jpg",
	"cwd_up.jpg",
	"cwd_dn.jpg",
	"cwd_ft.jpg",
	"cwd_bk.jpg"
};

const int i = 16; 
GLuint VAO[i], VBO[i*3], VTO[i];

ModelData mesh_data1, mesh_data2, mesh_data3, mesh_data4, mesh_data5, mesh_data6, mesh_data7, mesh_data8, mesh_data9, mesh_data10, mesh_data11, mesh_data12, mesh_data13;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y= 0.0f;

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertexShader, const char* fragmentShader)
{
	GLuint shaderProgramID;
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertexShader, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragmentShader, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

GLuint loadCubemap(vector<std::string>faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else {
			cout << "Cubemap not loaded at: " << faces[i];
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	
	// Models
	mesh_data1 = load_mesh(MESH_NAME);
	mesh_data2 = load_mesh(MESH_NAME2);
	mesh_data3 = load_mesh(MESH_NAME3);
	mesh_data4 = load_mesh(MESH_NAME4);
	mesh_data5 = load_mesh(MESH_NAME5);
	mesh_data6 = load_mesh(MESH_NAME6);
	mesh_data7 = load_mesh(MESH_NAME7);
	mesh_data8 = load_mesh(MESH_NAME8);
	mesh_data9 = load_mesh(MESH_NAME9);
	mesh_data10 = load_mesh(MESH_NAME10);
	mesh_data11 = load_mesh(MESH_NAME11);
	mesh_data12 = load_mesh(MESH_NAME12);
	mesh_data13 = load_mesh(MESH_NAME13);

	loc1 = glGetAttribLocation(sunShader, "vertex_position");
	loc2 = glGetAttribLocation(sunShader, "vertex_normal");
	loc3 = glGetAttribLocation(sunShader, "vertex_texture");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Textures
	int width, height, nrChannels;
	unsigned char *data;

	// ---------------------------------------- SUN ----------------------------------------
	glGenTextures(1, &VTO[0]);
	glBindTexture(GL_TEXTURE_2D, VTO[0]);

	data = stbi_load(sun, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mVertices[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &VBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec2), &mesh_data1.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[0]);
	glBindVertexArray(VAO[0]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- MERCURY ----------------------------------------
	glGenTextures(1, &VTO[1]);
	glBindTexture(GL_TEXTURE_2D, VTO[1]);

	data = stbi_load(mercury, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec2), &mesh_data2.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[1]);
	glBindVertexArray(VAO[1]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- VENUS ----------------------------------------
	glGenTextures(1, &VTO[2]);
	glBindTexture(GL_TEXTURE_2D, VTO[2]);

	data = stbi_load(venus, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec2), &mesh_data3.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[2]);
	glBindVertexArray(VAO[2]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	
	// ---------------------------------------- EARTH ----------------------------------------
	glGenTextures(1, &VTO[3]);
	glBindTexture(GL_TEXTURE_2D, VTO[3]);

	data = stbi_load(earth, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec3), &mesh_data4.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec3), &mesh_data4.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[11]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec2), &mesh_data4.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[3]);
	glBindVertexArray(VAO[3]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[11]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- MOON ----------------------------------------
	glGenTextures(1, &VTO[4]);
	glBindTexture(GL_TEXTURE_2D, VTO[4]);

	data = stbi_load(moon, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[12]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[12]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data5.mPointCount * sizeof(vec3), &mesh_data5.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[13]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[13]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data5.mPointCount * sizeof(vec3), &mesh_data5.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[14]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[14]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data5.mPointCount * sizeof(vec2), &mesh_data5.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[4]);
	glBindVertexArray(VAO[4]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[12]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[13]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[14]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- MARS ----------------------------------------
	glGenTextures(1, &VTO[5]);
	glBindTexture(GL_TEXTURE_2D, VTO[5]);

	data = stbi_load(mars, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[15]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[15]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data6.mPointCount * sizeof(vec3), &mesh_data6.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[16]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[16]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data6.mPointCount * sizeof(vec3), &mesh_data6.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[17]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[17]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data6.mPointCount * sizeof(vec2), &mesh_data6.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[5]);
	glBindVertexArray(VAO[5]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[15]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[16]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[17]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- JUPITER ----------------------------------------
	glGenTextures(1, &VTO[6]);
	glBindTexture(GL_TEXTURE_2D, VTO[6]);

	data = stbi_load(jupiter, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[18]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[18]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data7.mPointCount * sizeof(vec3), &mesh_data7.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[19]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[19]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data7.mPointCount * sizeof(vec3), &mesh_data7.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[20]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[20]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data7.mPointCount * sizeof(vec2), &mesh_data7.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[6]);
	glBindVertexArray(VAO[6]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[18]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[19]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[20]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- SATURN ----------------------------------------
	glGenTextures(1, &VTO[7]);
	glBindTexture(GL_TEXTURE_2D, VTO[7]);

	data = stbi_load(saturn, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[21]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[21]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data8.mPointCount * sizeof(vec3), &mesh_data8.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[22]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[22]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data8.mPointCount * sizeof(vec3), &mesh_data8.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[23]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[23]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data8.mPointCount * sizeof(vec2), &mesh_data8.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[7]);
	glBindVertexArray(VAO[7]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[21]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[22]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[23]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- SATURN_RING ----------------------------------------
	glGenBuffers(1, &VBO[24]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[24]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data9.mPointCount * sizeof(vec3), &mesh_data9.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[25]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[25]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data9.mPointCount * sizeof(vec3), &mesh_data9.mNormals[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[8]);
	glBindVertexArray(VAO[8]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[24]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[25]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- URANUS ----------------------------------------
	glGenTextures(1, &VTO[8]);
	glBindTexture(GL_TEXTURE_2D, VTO[8]);

	data = stbi_load(uranus, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[26]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[26]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data10.mPointCount * sizeof(vec3), &mesh_data10.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[27]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[27]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data10.mPointCount * sizeof(vec3), &mesh_data10.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[28]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[28]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data10.mPointCount * sizeof(vec2), &mesh_data10.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[9]);
	glBindVertexArray(VAO[9]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[26]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[27]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[28]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- NEPTUNE ----------------------------------------
	glGenTextures(1, &VTO[9]);
	glBindTexture(GL_TEXTURE_2D, VTO[9]);

	data = stbi_load(neptune, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[29]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[29]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data11.mPointCount * sizeof(vec3), &mesh_data11.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[30]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[30]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data11.mPointCount * sizeof(vec3), &mesh_data11.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[31]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[31]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data11.mPointCount * sizeof(vec2), &mesh_data11.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[10]);
	glBindVertexArray(VAO[10]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[29]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[30]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[31]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- PLUTO ----------------------------------------
	glGenTextures(1, &VTO[10]);
	glBindTexture(GL_TEXTURE_2D, VTO[10]);

	data = stbi_load(pluto, &width, &height, &nrChannels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glGenBuffers(1, &VBO[32]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[32]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data12.mPointCount * sizeof(vec3), &mesh_data12.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[33]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[33]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data12.mPointCount * sizeof(vec3), &mesh_data12.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[34]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[34]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data12.mPointCount * sizeof(vec2), &mesh_data12.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[11]);
	glBindVertexArray(VAO[11]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[32]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[33]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[34]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// ---------------------------------------- SPACESHIP ----------------------------------------
	glGenBuffers(1, &VBO[35]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[35]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data13.mPointCount * sizeof(vec3), &mesh_data13.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[36]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[36]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data13.mPointCount * sizeof(vec3), &mesh_data13.mNormals[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[12]);
	glBindVertexArray(VAO[12]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[35]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[36]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera
	glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(camera - cameraTarget);

	glm::vec3 cameraRight = glm::normalize(cross(up, cameraDirection));
	glm::mat4 view = glm::lookAt(camera, camera + cameraFront, up);

	mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	// ---------------------------------------- SUN ----------------------------------------
	glUseProgram(sunShader);
	glBindTexture(GL_TEXTURE_2D, VTO[0]);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(sunShader, "model");
	int view_mat_location = glGetUniformLocation(sunShader, "view");
	int proj_mat_location = glGetUniformLocation(sunShader, "proj");
	int camera_location = glGetUniformLocation(sunShader, "camera");

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniform3fv(camera_location, 1, glm::value_ptr(camera));

	glBindVertexArray(VAO[0]);

	mat4 sun = identity_mat4();
	sun = rotate_y_deg(sun, rotate_y);

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, sun.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data1.mPointCount);

	// ---------------------------------------- MERCURY ----------------------------------------
	glUseProgram(planetShader);
	glBindTexture(GL_TEXTURE_2D, VTO[1]);

	//Declare your uniform variables that will be used in your shader
	matrix_location = glGetUniformLocation(planetShader, "model");
	view_mat_location = glGetUniformLocation(planetShader, "view");
	proj_mat_location = glGetUniformLocation(planetShader, "proj");

	//update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));

	glBindVertexArray(VAO[1]);

	mat4 mercury = identity_mat4();
	mercury = rotate_y_deg(mercury, rotate_y);
	mercury = translate(mercury, vec3(-10.0f, 0.0f, 2.0f));

	mercury = sun * mercury;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, mercury.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);

	// ---------------------------------------- VENUS ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[2]);

	glBindVertexArray(VAO[2]);

	mat4 venus = identity_mat4();
	venus = rotate_y_deg(venus, rotate_y);
	venus = translate(venus, vec3(10.0f, 0.7f, 4.0f));

	venus = sun * venus;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, venus.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data3.mPointCount);

	// ---------------------------------------- EARTH ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[3]);

	glBindVertexArray(VAO[3]);

	mat4 earth = identity_mat4();
	earth = rotate_y_deg(earth, rotate_y);
	earth = translate(earth, vec3(13.0f, 0.3f, 0.0f));
	
	earth = sun * earth;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, earth.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data4.mPointCount);

	// ---------------------------------------- MOON ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[4]);

	glBindVertexArray(VAO[4]);

	mat4 moon = identity_mat4();
	moon = translate(moon, vec3(-1.0f, 1.0f, 0.0f));
	moon = rotate_z_deg(moon, rotate_y);

	moon = earth * moon;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, moon.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data5.mPointCount);

	// ---------------------------------------- MARS ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[5]);

	glBindVertexArray(VAO[5]);

	mat4 mars = identity_mat4();
	mars = rotate_y_deg(mars, rotate_y);
	mars = translate(mars, vec3(14.0f, -0.5f, -5.0f));

	mars = sun * mars;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, mars.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data6.mPointCount);

	// ---------------------------------------- JUPITER ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[6]);

	glBindVertexArray(VAO[6]);

	mat4 jupiter = identity_mat4();
	jupiter = rotate_y_deg(jupiter, rotate_y);
	jupiter = translate(jupiter, vec3(-15.0f, 1.0f, -6.0f));

	jupiter = sun * jupiter;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, jupiter.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data7.mPointCount);

	// ---------------------------------------- SATURN ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[7]);

	glBindVertexArray(VAO[7]);

	mat4 saturn = identity_mat4();
	saturn = rotate_y_deg(saturn, rotate_y);
	saturn = translate(saturn, vec3(19.0f, -0.4f, -8.0f));

	saturn = sun * saturn;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, saturn.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data8.mPointCount);

	// ---------------------------------------- SATURN_RING ----------------------------------------
	glBindVertexArray(VAO[8]);

	mat4 rings = identity_mat4();
	rings = rotate_x_deg(rings, rotate_y);

	rings = saturn * rings;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, rings.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data9.mPointCount);

	// ---------------------------------------- URANUS ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[8]);

	glBindVertexArray(VAO[9]);

	mat4 uranus = identity_mat4();
	uranus = rotate_y_deg(uranus, rotate_y);
	uranus = translate(uranus, vec3(-22.0f, 0.0f, 0.0f));

	uranus = sun * uranus;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, uranus.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data10.mPointCount);

	// ---------------------------------------- NEPTUNE ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[9]);

	glBindVertexArray(VAO[10]);

	mat4 neptune = identity_mat4();
	neptune = rotate_y_deg(neptune, rotate_y);
	neptune = translate(neptune, vec3(-24.0f, -0.6f, 16.0f));

	neptune = sun * neptune;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, neptune.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data11.mPointCount);

	// ---------------------------------------- PLUTO ----------------------------------------
	glBindTexture(GL_TEXTURE_2D, VTO[10]);

	glBindVertexArray(VAO[11]);

	mat4 pluto = identity_mat4();
	pluto = rotate_y_deg(pluto, rotate_y);
	pluto = translate(pluto, vec3(-25.0f, 0.3f, -17.5f));

	pluto = sun * pluto;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, pluto.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data12.mPointCount);

	// ---------------------------------------- SPACESHIP ----------------------------------------
	glBindVertexArray(VAO[12]);

	mat4 spaceship = identity_mat4();
	spaceship = translate(spaceship, vec3(-15.0f, 0.0f, 15.0f));
	spaceship = rotate_y_deg(spaceship, rotate_y);
	spaceship = rotate_z_deg(spaceship, rotate_y);

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, spaceship.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data13.mPointCount);

	glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 20.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	sunShader = CompileShaders("./assets/shaders/sunVertexShader.txt", "./assets/shaders/sunFragmentShader.txt");
	planetShader = CompileShaders("./assets/shaders/planetVertexShader.txt", "./assets/shaders/planetFragmentShader.txt");
	generateObjectBufferMesh();
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
}

void specialKeys(int key, int x, int y) {
	float cameraSpeed = 2.0f;
	switch (key) {
	case GLUT_KEY_UP: 
		camera += cameraFront * cameraSpeed;
		break;
	case GLUT_KEY_DOWN:
		camera -= cameraFront * cameraSpeed;
		break;
	case GLUT_KEY_RIGHT:
		camera += glm::normalize(cross(cameraFront, up)) * cameraSpeed;
		break;
	case GLUT_KEY_LEFT:
		camera -= glm::normalize(cross(cameraFront, up)) * cameraSpeed;
		break;
	}
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Space");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialKeys);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
