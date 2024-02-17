#version 330 core

out vec4 color;

uniform vec3 textColor = vec3(1.0, 1.0, 1.0);
float opacity = 1.0;


void main() {
    color = vec4(textColor, opacity);
};

