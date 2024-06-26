# Raylib GPU Particles

<img align="left" style="width:260px" src="https://github.com/arceryz/raylib-gpu-particles/blob/master/demo.gif" width="288px">

**This example creates a 3D particle system fully on the GPU**.

On a simple laptop (MSI Katana) you can draw 2 million particles per frame at 60 fps. The particles are rendered as triangles with direction and magnitude.

The code is **Highly** documented, you will absolutely understand! It uses Raylib, rlgl (lower level Raylib) and Raygui. You can learn GPU Instancing, particle billboarding and rotating to movement and compute shaders all in Raylib!

---
This example requires raylib to be compiled with `GRAPHIC_API=GRAPHICS_API_OPENGL_43`, otherwise you won't be able to use compute shaders.

Features:

- **Highly** documented, you will absolutely understand!
- 100% GPU updated and drawn particles.
- `raylib.h` API.
- `rlgl.h` lower level API.
- `raygui.h` API for **fun** sliders and playing around.
- GPU Instancing example.
- Particle billboarding and rotating to movement example.
- Compute shader example.

## Usage
There are four files in this project. The shaders should be placed under the /Shaders subdirectory. Make sure you have `raygui.h` in your source.

- `main.c`
- `particle_compute.glsl`: Update the particles positions and velocity.
- `particle_vertex.glsl`: Convert triangles to particles by rotating, scaling, coloring and facing them to the camera.
- `particle_fragment.glsl`: Output color.

The simulating parameters can be modified with sliders provided by RayGUI.