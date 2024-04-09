#version 430

// This is the vertex position of the base particle!
// This is the vertex attribute set in the code, index 0.
layout (location=0) in vec3 vertexPosition;

// Input uniform values.
layout (location=0) uniform mat4 projectionMatrix;
layout (location=1) uniform mat4 viewMatrix;
layout (location=2) uniform float particleScale;

// The two buffers we will be reading from.
// We can write to them here but should not.
layout(std430, binding=0) buffer ssbo0 { vec4 positions[]; };
layout(std430, binding=1) buffer ssbo1 { vec4 velocities[]; };

// We will only output color.
out vec4 fragColor;

void main()
{
    vec3 velocity = velocities[gl_InstanceID].xyz;
    vec3 position = positions[gl_InstanceID].xyz;

    // Set color to a nice gradient depending on direction.
    fragColor.rgb = abs(normalize(velocity)) + 0.2;
    fragColor.a = 1.0;

    // We want to do two things:
    // 1. Make the particle face the camera.
    // 2. Make the particle point to its direction of movement.
    //
    // Point (1) we will achieve here by not multiplying by the view matrix,
    // since the view matrix will rotate the vertex. We only need the translation from it.
    // Therefore will add view-space world position at the end.
    float scale = 0.005*particleScale;
    vec3 vertexView = vertexPosition*scale;

    // With the triangle facing the camera, we want it to now point in the
    // direction of its movement (in view space).
    // First we compute the angle of the velocity in view space.
    vec2 velocityView = (viewMatrix * vec4(velocity, 0)).xy;
    float velocityAngle = atan(velocityView.y, velocityView.x);
    float speed = length(velocityView);

    // Our triangle's tip is currently at 90 degrees in view space.
    // To make it point to velocityAngle, we rotate by (90-angle) degrees backwards (-1x).
    float rot = velocityAngle - radians(90);

    // These are the two vectors for a clockwise rotation.
    // We perform essentially 2x2 matrix multiplication.
    vec2 xvec = vec2(cos(rot), sin(rot));
    vec2 yvec = vec2(-sin(rot), cos(rot));
    vertexView.xy = vertexView.x*xvec + vertexView.y*yvec;

    // We scale the tip of the vertex by checking if gl_VertexID==2.
    // We avoid if* statements.
    float isTip = float(gl_VertexID == 2);
    float arrowLength = speed*0.05;
    vertexView.xy = vertexView.xy*(1-isTip) + isTip*vertexView.xy * (arrowLength+1);

    // Add the particle position to the vertex (in view space).
    vertexView += (viewMatrix * vec4(position, 1)).xyz;

    // Calculate final vertex position.
    gl_Position = projectionMatrix * vec4(vertexView, 1);
}