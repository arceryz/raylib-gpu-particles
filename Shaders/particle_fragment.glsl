#version 430

in vec4 fragColor;
out vec4 finalColor;

void main()
{
    // There is only one thing to do.
    finalColor = fragColor;
}