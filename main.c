#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <stdlib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

float GetRandomFloat(float from, float to) {
    return from + (to-from)*(float)GetRandomValue(0, INT_MAX) / INT_MAX;
}

int main() {
    InitWindow(800, 800, "GPU Particles");

    // Compute shader for updating particles.
    char *shaderCode = LoadFileText("Shaders/particle_compute.glsl");
    int shaderData = rlCompileShader(shaderCode, RL_COMPUTE_SHADER);
    int computeShader = rlLoadComputeShaderProgram(shaderData);
    UnloadFileText(shaderCode);

    // Shader for constructing triangles and drawing.
    Shader particleShader = LoadShader(
        "Shaders/particle_vertex.glsl", 
        "Shaders/particle_fragment.glsl");


    //
    // Now we prepare the buffers that we connect to the shaders.
    // For each variable we want to give our particles, we create one buffer
    // called a Shader Storage Buffer Object containing a single variable type.
    //
    // We will use only Vector4 as particle variables, because data in buffers
    // requires very strict alignment rules.
    // You can send structs, but if not properly aligned will introduce many bugs.
    // For information on the std430 buffer layout see:
    // https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL).
    //
    // Number of particles should be a multiple of 1024, our workgroup size (set in shader).
    int numParticles = 1024*100;
    Vector4 *positions = RL_MALLOC(sizeof(Vector4[numParticles]));
    Vector4 *velocities = RL_MALLOC(sizeof(Vector4[numParticles]));

    for (int i = 0; i < numParticles; i++) {
        // We only use the XYZ components of position and velocity.
        // Use the remainder for extra effects if needed, or create more buffers.
        positions[i] = (Vector4){ 
            GetRandomFloat(-0.5, 0.5),
            GetRandomFloat(-0.5, 0.5),
            GetRandomFloat(-0.5, 0.5),
            0,
        }; 
        velocities[i] = (Vector4){ 0, 0, 0, 0 };
    }

    // Load three buffers: Position, Velocity and Starting Position. Read/Write=RL_DYNAMIC_COPY.
    int ssbo0 = rlLoadShaderBuffer(numParticles*sizeof(Vector4), positions, RL_DYNAMIC_COPY);
    int ssbo1 = rlLoadShaderBuffer(numParticles*sizeof(Vector4), velocities, RL_DYNAMIC_COPY);
    int ssbo2 = rlLoadShaderBuffer(numParticles*sizeof(Vector4), positions, RL_DYNAMIC_COPY);

    // For instancing we need a Vertex Array Object. 
    // Raylib Mesh* is inefficient for millions of particles.
    // For info see: https://www.khronos.org/opengl/wiki/Vertex_Specification
    int particleVao = rlLoadVertexArray();
    rlEnableVertexArray(particleVao);

    // Our base particle mesh is a triangle on the unit circle.
    // We will rotate and stretch the triangle in the vertex shader.
    Vector3 vertices[] = {
        { -0.86, -0.5, 0.0 },
        { 0.86, -0.5, 0.0 },
        { 0.0f,  1.0f, 0.0f }
    };

    // Configure the vertex array with a single attribute of vec3.
    // This is the input to the vertex shader.
    rlLoadVertexBuffer(vertices, sizeof(vertices),  false); // dynamic=false
    rlEnableVertexAttribute(0);
    rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
    rlDisableVertexArray(); // Stop editing.

    Camera camera = { { 2, 2, 2 }, {}, { 0, 1, 0 }, 35.0, CAMERA_PERSPECTIVE };
    float time = 0;
    float timeScale = 0.2f;
    float sigma = 10;
    float rho = 28;
    float beta = 8.0/3.0;
    float particleScale = 1.0;
    float instances_x1000 = 100.0;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        int numInstances = (int)(instances_x1000/1000*numParticles);
        UpdateCamera(&camera, CAMERA_ORBITAL);

        { // Compute Pass.
            rlEnableShader(computeShader);

            // Set our parameters. The indices are set in the shader.
            rlSetUniform(0, &time, SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(1, &timeScale, SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(2, &deltaTime, SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(3, &sigma, SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(4, &rho, SHADER_UNIFORM_FLOAT, 1);
            rlSetUniform(5, &beta, SHADER_UNIFORM_FLOAT, 1);

            rlBindShaderBuffer(ssbo0, 0);
            rlBindShaderBuffer(ssbo1, 1);
            rlBindShaderBuffer(ssbo2, 2);

            // We have numParticles/1024 workGroups. Each workgroup has size 1024.
            rlComputeShaderDispatch(numParticles/1024, 1, 1);
            rlDisableShader();
        }

        BeginDrawing();
        ClearBackground(BLACK); 

        { // Render Pass.
            BeginMode3D(camera);
            rlEnableShader(particleShader.id);

            // Because we use rlgl, we must take care of matrices ourselves.
            // We need to only pass the projection and view matrix.
            // We also pass the inverse view matrix for aligning particles to the camera.
            Matrix projection = rlGetMatrixProjection();
            Matrix view = GetCameraMatrix(camera);
            Matrix inverseView = MatrixInvert(view);

            SetShaderValueMatrix(particleShader, 0, projection);
            SetShaderValueMatrix(particleShader, 1, view);
            SetShaderValueMatrix(particleShader, 2, inverseView);
            SetShaderValue(particleShader, 3, &particleScale, SHADER_UNIFORM_FLOAT);

            rlBindShaderBuffer(ssbo0, 0);
            rlBindShaderBuffer(ssbo1, 1);

            // Draw the particles. Instancing will duplicate the vertices.
            rlEnableVertexArray(particleVao);
            rlDrawVertexArrayInstanced(0, 3, numInstances);
            rlDisableVertexArray(); 
            rlDisableShader();

            DrawCubeWires((Vector3){}, 1.0, 1.0, 1.0, DARKGRAY);
            EndMode3D();
        }

        { // GUI Pass.
            GuiSlider((Rectangle){ 550, 10, 200, 10 }, "Particles x1000", TextFormat("%.2f", instances_x1000), &instances_x1000, 0, 1000);
            GuiSlider((Rectangle){ 550, 25, 200, 10 }, "Particle Scale", TextFormat("%.2f", particleScale), &particleScale, 0, 5);
            GuiSlider((Rectangle){ 550, 40, 200, 10 }, "Speed", TextFormat("%.2f", timeScale), &timeScale, 0, 1.0);
            GuiSlider((Rectangle){ 650, 70, 100, 10 }, "Sigma", TextFormat("%2.1f", sigma), &sigma, 0, 20);
            GuiSlider((Rectangle){ 650, 85, 100, 10 }, "Rho", TextFormat("%2.1f", rho), &rho, 0, 30);
            GuiSlider((Rectangle){ 650, 100, 100, 10 }, "Beta", TextFormat("%2.1f", beta), &beta, 0, 10);

            time += deltaTime;
            if (GuiButton((Rectangle){ 350, 10, 100, 20 }, "Restart (Space)") || IsKeyPressed(KEY_SPACE)) {
                time = 0;
            }
            if (GuiButton((Rectangle){ 280, 10, 60, 20 }, "Reset")) {
                time = 0;
                timeScale = 0.2f;
                sigma = 10;
                rho = 28;
                beta = 8.0/3.0;
                particleScale = 1.0;
                instances_x1000 = 100.0;
            }

            DrawFPS(10, 10);
            DrawText(TextFormat("N=%d", numInstances), 10, 30, 20, DARKGRAY);
        }

        EndDrawing();
    }   
}