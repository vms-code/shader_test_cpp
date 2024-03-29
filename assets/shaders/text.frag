#version 330
// alpha_discard.fs from Raylib

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

uniform vec3 fragColor = vec3(1.0, 1.0, 1.0);
float opacity = 1.0;

// Input uniform values
uniform sampler2D texture0;
//uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a == 0.0) discard;
    finalColor = texelColor * fragColor * opacity;
}
