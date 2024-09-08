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

uniform float fog_factor;

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

    vec4 color = vec4(0.0);

    if (colDiffuse.r == 0 && colDiffuse.g == 0 && colDiffuse.b == 1) {
        color = window_color;
    } else {
        color = texelColor * fragColor * colDiffuse;
    }

    // Fog calculation
    float dist = length(viewPos.xz - fragPosition.xz);
    vec4 fogColor = vec4(1,1,1,1);


    // fog_factor = 1.0/28.0
    // lower value of fog_factor gives more visibility.

    dist = clamp(dist * fog_factor, 0, 1);

    // hack
    float alpha = 1;
    if (texelColor.a == 0)
        alpha = 0;

    //finalColor = vec4(color.rgb, color.a * (1-dist));
    finalColor = vec4(mix(color.rgb, fogColor.rgb, dist), alpha);
}
