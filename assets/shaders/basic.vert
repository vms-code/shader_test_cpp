#version 330 core

attribute vec3 position;

uniform mat4 projection;
uniform mat4 modelView; // view matrix * model matrix

void main() {


    vec4 point_position = vec4( position, 1.0 );

    gl_Position = projection * modelView * point_position;
}


