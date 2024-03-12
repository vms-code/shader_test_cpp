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

    vec3 pos = position;
    // billboarding
    // Quad billboard: Works only on quads that have its center at origin.
    // http://www.songho.ca/opengl/files/gl_anglestoaxes01.png
    vec3 right = vec3( modelView[0][0], modelView[1][0], modelView[2][0]),
        up = vec3( modelView[0][1], modelView[1][1], modelView[2][1] );

    // Rotate vertex toward camera
    pos = (right * pos.x) - (up * pos.y);

    vec4 point_position = vec4( pos, 1.0 );
    // Calculate final vertex position
    gl_Position = projection * modelView * point_position;
    //gl_Position = projection * point_position;
}
