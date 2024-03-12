#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
//in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;

uniform vec3 fragTextColor = vec3(0.0, 1.0, 0.0);  // Default color is green
uniform float opacity = 1.0;  // Default opacity is 1.0

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    //vec4 textColor = vec4(1.0, 0.0, 0.0, 1.0); // Red color

    vec4 diffuseColor = vec4( fragTextColor, opacity );

    // Texel color fetching from texture sampler
    // NOTE: Calculate alpha using signed distance field (SDF)
    float distanceFromOutline = texture(texture0, fragTexCoord).a - 0.5;
    float distanceChangePerFragment = length(vec2(dFdx(distanceFromOutline), dFdy(distanceFromOutline)));
    float alpha = smoothstep(-distanceChangePerFragment, distanceChangePerFragment, distanceFromOutline);

    // Calculate final fragment color
    finalColor = vec4(diffuseColor.rgb, diffuseColor.a*alpha);
    //finalColor = vec4(textColor.rgb, textColor.a*alpha);
    //vec4 texelColor = texture(texture0, fragTexCoord);
    //finalColor = texelColor * textColor;
}
