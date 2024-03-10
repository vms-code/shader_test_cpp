#version 330

// Input vertex attributes
in vec3 position;
in vec2 vertexTexCoord;
//in vec4 vertexColor;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
//out vec4 fragColor;

// Input uniform values
uniform mat4 projection;
uniform mat4 modelView; // view matrix * model matrix

void main() {
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    //fragColor = vertexColor;

    vec4 point_position = vec4( position, 1.0 );

    // Calculate final vertex position
    gl_Position = projection * modelView * point_position;
    //gl_Position = projection * point_position;
}
