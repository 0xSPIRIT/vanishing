precision mediump float;
precision mediump int;

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float time;

// Output fragment color
out vec4 finalColor;

void main() {
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    if (texelColor.a == 0.0) {
        discard;
    }

    if (colDiffuse == vec4(1.0,0.0,0.0,1.0)) {
        discard;
    }
    if (colDiffuse == vec4(1.0,0.0,1.0,1.0)) {
        finalColor = vec4(1.0,1.0,1.0,1.0);
        return;
    }

    float factor = 1.0;

    float dist = factor * 0.005f * length(fragPosition) - (0.003 * fragPosition.y);
    //dist *= dist;

    dist = clamp(dist, 0.0, 1.0);

    finalColor = texelColor * fragColor * colDiffuse;

    vec4 sky = vec4(1.0, 241.0/255.0, 186.0/255.0, 1.0);
    finalColor = mix(finalColor, sky, dist);
}


