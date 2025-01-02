precision mediump float;

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
    if (colDiffuse == vec4(1.0,0.0,0.0,1.0)) {
        discard;
    }

    float dist = 0.05 * length(fragPosition + vec3(20.0,0.0,-10.0));

    if (dist > 1.0)
        finalColor = vec4(0.0,0.0,0.0,1.0);
    else
        finalColor = mix(texelColor, vec4(0.0,0.0,0.0,1.0), dist);
}

