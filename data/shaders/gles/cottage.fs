#version 300 es

precision mediump float;

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor; // we don't really use vertex colors

out vec4 finalColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    if (colDiffuse == vec4(1.0,0.0,0.0,1.0)) {
        discard;
    }

    if (texelColor.a == 0.0) {
        discard;
    }

    finalColor = texelColor*colDiffuse;
}
