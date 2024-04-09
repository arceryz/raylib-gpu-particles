// We require version 430 since it supports compute shaders.
#version 430

// This is the workgroup size. The largest size that is guaranteed by OpenGL 
// to available is 1024, beyond this is uncertain.
// Might influence performance but only in advanced cases.
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

//
// Shader Storage Buffer Objects (SSBO's) can store any data and are
// declared like structs. There can only be one variable-sized array
// in an SSBO, for this reason we use multiple SSBO's to store our arrays.
//
// We do not use structures to store the particles, because an SSBO
// only has a guaranteed max size of 128 Mb. 
// With 1 million particles you can store only 32 floats per particle in one SSBO.
// Therefore use multiple buffers since this example scales up to the millions.
//
layout(std430, binding=0) buffer ssbo0 { vec4 positions[]; };
layout(std430, binding=1) buffer ssbo1 { vec4 velocities[]; };
layout(std430, binding=2) buffer ssbo2 { vec4 startPositions[]; };

// Uniform values are the way in which we can modify the shader efficiently.
// These can be updated every frame efficiently.
// We use layout(location=...) but you can also leave it and query the location in Raylib.
layout(location=0) uniform float time;
layout(location=1) uniform float timeScale;
layout(location=2) uniform float deltaTime;

// These are some particle simulation parameters.
// You can turn these into uniforms if you want to modify them at runtime.
layout(location=3) uniform float sigma;
layout(location=4) uniform float rho;
layout(location=5) uniform float beta;

const float PI = 3.14159;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    vec3 pos = positions[index].xyz;

    // We reset the position when time is exactly zero.
    if (time == 0) {
        pos = startPositions[index].xyz;
    }

    // Since our particles are in the (-1, 1) space
    // we transform the position to be suitable for the Lorenz system.
    pos = 30*pos + vec3(0, 0, 30);

    // This is the Lorenz system (See wikipedia). Velocity=dx/dt.
    vec3 vel = vec3(0.0);
    vel.x = sigma*(pos.y - pos.x);
    vel.y = pos.x*(rho - pos.z) - pos.y;
    vel.z = pos.x*pos.y - beta*pos.z;

    // Update and transform back to (-1, 1).
    pos += vel*deltaTime*timeScale;
    pos = (pos - vec3(0, 0, 30)) / 30;

    // Assign the new values to the buffers.
    positions[index].xyz = pos;
    velocities[index].xyz = vel;
}