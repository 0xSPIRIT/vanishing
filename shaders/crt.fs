#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float time;

uniform int do_scanline_effect;

out vec4 finalColor;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 tex_coord = fragTexCoord;

    float scan_intensity = 0.5;
    float noise_intensity = 0.25;
    float abberation_intensity = 1;

    const float pi = 3.14159265f;

    float sine_frequency = 100 + (scan_intensity-1) * 0.01;
    float sine_amplitude = 0.0002 + (scan_intensity-1) * 0.0001;

    // increase amplitude to top and bottom

    float t = 900 + mod(time, 10);

    float t_off = t * 20 * scan_intensity;

    // sine wave to entire scene
    if (do_scanline_effect == 1) {
        tex_coord.x += sine_amplitude * sin(t_off + tex_coord.y * sine_frequency);

        float scanline_frequency = 4 * scan_intensity;
        float scanline_amplitude = 0.0005 * scan_intensity;
        float scanline_speed = 2 * scan_intensity * sqrt(scan_intensity);
        tex_coord.x += scanline_amplitude * tan(t * scanline_speed + scanline_frequency * tex_coord.y);
    }

    if (abberation_intensity > 0) {
        float redOffset   =  0.009;
        float greenOffset =  0.006;
        float blueOffset  = -0.006;

        vec2 dir = vec2(0.32,0.32);
        dir *= abberation_intensity;

        finalColor.r  = texture(texture0, tex_coord + dir * vec2(redOffset)).r;
        finalColor.g  = texture(texture0, tex_coord + dir * vec2(greenOffset)).g;
        finalColor.ba = texture(texture0, tex_coord + dir * vec2(blueOffset)).ba;
    } else {
        finalColor = texture(texture0, tex_coord);
    }

    // Vignette
    float dist = 1;
    dist = 0.6 - length(tex_coord.xy - vec2(0.5, 0.5));
    //dist = sqrt(dist);
    dist *= dist * 8;
    dist += 0.3;

    finalColor.r = clamp(finalColor.r * dist, 0, 1);
    finalColor.g = clamp(finalColor.g * dist, 0, 1);
    finalColor.b = clamp(finalColor.b * dist, 0, 1);

    float x = mod(fragTexCoord.x * 0.005 + t/2, 100);
    float y = mod(fragTexCoord.y * 0.006 + t*2, 100);

    // Actual scanlines
    if (do_scanline_effect == 1) {
        float y = tex_coord.y + time * 0.25;
        int scanline_count = 80;
        int y_i = int(round(y * scanline_count * 1));

        if (y_i % 2 == 0) {
            finalColor = mix(finalColor, vec4(0,0,0,1), 0.25);
        }
    }
}

