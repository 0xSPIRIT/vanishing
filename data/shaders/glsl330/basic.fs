#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;

void main() {
    if (colDiffuse.rgb == vec3(1,0,0))
        discard;

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);

    vec3 light_intensity = vec3(0.0);

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 1) {
            float dist = length(lights[i].position - fragPosition);
            light_intensity += lights[i].color.rgb * pow(0.92, dist);
        }
    }

    const float ambient = 0.10;
    light_intensity = max(light_intensity, ambient);

    finalColor = vec4(texelColor.rgb * colDiffuse.rgb * light_intensity, 1);
}
