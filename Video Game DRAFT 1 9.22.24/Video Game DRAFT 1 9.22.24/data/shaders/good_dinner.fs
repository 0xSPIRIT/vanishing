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

    vec4 white = vec4(32./255,32./255,32./255,1);//220.0/255.0, 220.0/255.0, 220.0/255.0, 1);

    float len = length(fragPosition - vec3(-23.59, 0, 9.148));
    float fac = max(0, min(1, 1 - 0.001f * len * len));

    finalColor = texelColor * fragColor * colDiffuse;
    finalColor = mix(finalColor, white, 1-fac);
}

