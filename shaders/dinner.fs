#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main() {
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Special color for the collision geometry so we don't draw it shhh
    if (colDiffuse == vec4(1,0,0,1)) {
        discard;
    }

    vec4 window_color = vec4(0.0, 0.0, 0.7, 0.5);

    if (colDiffuse.r == 0 && colDiffuse.g == 0 && colDiffuse.b == 1) {
        finalColor = window_color;
    } else {
        finalColor = texelColor * fragColor * colDiffuse;
    }
}
