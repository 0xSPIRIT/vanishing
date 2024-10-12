#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

// Output fragment color
out vec4 finalColor;

void main() {
    // Special color for the collision geometry so we don't draw it shhh
    if (colDiffuse == vec4(1,0,0,1)) {
        discard;
    }

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    float distance = length(viewPos);

    vec4 white = vec4(0.75,0.75,0.75,1);

    float fac = distance;

    //if (len == 0) discard;

    finalColor = texelColor * fragColor * colDiffuse;
    finalColor = vec4(mix(finalColor.rgb, white.rgb, fac), 1);
}

