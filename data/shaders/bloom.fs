precision mediump float;
precision mediump int;

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float time;

uniform float bloom_intensity;
uniform float vignette_mix;

uniform vec2 window_size;

out vec4 finalColor;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 vignette(vec2 tex_coord, vec4 col) {
    vec4 unprocessed = col;

    float dist = 1.0;
    dist = length(tex_coord.xy - vec2(0.5, 0.5));

    dist = 1.0-dist;

    col *= dist;
    col = mix(unprocessed, col, vignette_mix);

    const float noise_fac = 0.05;

    float t = sin(time);

    col.r += noise_fac * rand(tex_coord.xy+vec2(t,0.0));
    col.g += noise_fac * rand(tex_coord.xy+vec2(t,t));
    col.b += noise_fac * rand(tex_coord.xy-vec2(t,t));

    col.r = clamp(col.r, 0.0, 1.0);
    col.g = clamp(col.g, 0.0, 1.0);
    col.b = clamp(col.b, 0.0, 1.0);

    return col;
}

vec4 bloom(vec4 input_color, vec2 coords) {
    vec4 bloom = vec4(0.0);
    int j;
    int i;

    float bloomCoefficient = bloom_intensity;

    for( i = -6 ; i < 6; i++) {
        for (j = -5; j < 5; j++) {
            vec2 off = coords + vec2((float(j) / window_size.x), (float(i) / window_size.y));

            off.x = clamp(off.x, 0.001, 0.999);
            off.y = clamp(off.y, 0.001, 0.999);

            bloom += texture(texture0, off) * (bloomCoefficient * 0.01);
        }
    }

    bloom = bloom * bloom * 0.01;

    vec4 result = bloom + input_color;
    result.r = clamp(result.r, 0.0, 1.0);
    result.g = clamp(result.g, 0.0, 1.0);
    result.b = clamp(result.b, 0.0, 1.0);

    return result;
}

void main() {
    finalColor = texture(texture0, fragTexCoord);
    finalColor = bloom(finalColor, fragTexCoord);
    finalColor = vignette(fragTexCoord, finalColor);
}
