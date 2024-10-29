#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float time;

uniform int do_scanline_effect;
uniform float scanline_alpha;
uniform int do_warp_effect;
uniform float abberation_intensity;
uniform float vignette_intensity;
uniform float vignette_mix;

out vec4 finalColor;

vec2 clamp_vec2(vec2 a, vec2 min, vec2 max) {
    if (a.x < min.x) a.x = min.x;
    if (a.y < min.y) a.y = min.y;
    if (a.x > max.x) a.x = max.x;
    if (a.y > max.y) a.y = max.y;

    return a;
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 vignette(vec2 tex_coord, vec4 col) {
    vec4 unprocessed = col;

    if (vignette_intensity == 0) return col;

    float dist = 1;
    dist = length(tex_coord.xy - vec2(0.5, 0.5));
    dist *= 2;

    dist = 1-dist;
    dist *= 1.35f * vignette_intensity;

    if (do_scanline_effect == 1)
        dist *= (0.9 + 0.1 * rand(vec2(time, time)));

    if (vignette_intensity == 1)
        dist += 0.45;
    else
        dist = max(dist, 0.18);

    col.r = clamp(col.r * dist, 0, 1);
    col.g = clamp(col.g * dist, 0, 1);
    col.b = clamp(col.b * dist, 0, 1);

    col = mix(unprocessed, col, vignette_mix);

    return col;
}

void main() {
    vec2 tex_coord = fragTexCoord;

    float scan_intensity = 0.5;
    float noise_intensity = 0.25;

    const float pi = 3.14159265f;

    float sine_frequency = 100 + (scan_intensity-1) * 0.01;
    float sine_amplitude = 0.0002 + (scan_intensity-1) * 0.0001;

    // increase amplitude to top and bottom

    float t = 900 + mod(time, 10);

    float t_off = t * 20 * scan_intensity;

    // sine wave to entire scene
    if (do_warp_effect == 1) {
        tex_coord.x += sine_amplitude * sin(t_off + tex_coord.y * sine_frequency);

        float scanline_frequency = 4 * scan_intensity;
        float scanline_amplitude = 0.0005 * scan_intensity;
        float scanline_speed = 2 * scan_intensity * sqrt(scan_intensity);
        tex_coord.x += scanline_amplitude * tan(t * scanline_speed + scanline_frequency * tex_coord.y);
    }

    if (abberation_intensity > 0) {
        float redOffset   = -0.009;
        float greenOffset =  0.006;
        float blueOffset  = -0.006;

        vec2 dir = vec2(0.32,0.32);
        //dir *= abberation_intensity;

        vec4 abberated = vec4(1.0);

        vec2 r_coord = tex_coord + dir * vec2(redOffset);
        vec2 g_coord = tex_coord + dir * vec2(greenOffset);
        vec2 b_coord = tex_coord + dir * vec2(blueOffset);

        r_coord = clamp_vec2(r_coord, vec2(0.0, 0.0), vec2(1.0, 1.0));
        g_coord = clamp_vec2(g_coord, vec2(0.0, 0.0), vec2(1.0, 1.0));
        b_coord = clamp_vec2(b_coord, vec2(0.0, 0.0), vec2(1.0, 1.0));

        abberated.r  = texture(texture0, r_coord).r;
        abberated.g  = texture(texture0, g_coord).g;
        abberated.ba = texture(texture0, b_coord).ba;

        finalColor = mix(texture(texture0, tex_coord), abberated, abberation_intensity);
    } else {
        finalColor = texture(texture0, tex_coord);
    }

    // Vignette
    finalColor = vignette(tex_coord, finalColor);

    float x = mod(fragTexCoord.x * 0.005 + t/2, 100);
    float y = mod(fragTexCoord.y * 0.006 + t*2, 100);

    // Actual scanlines
    if (do_scanline_effect == 1) {
        float y = tex_coord.y + time * 0.25;
        int scanline_count = 80;
        int y_i = int(round(y * scanline_count * 1));

        if (y_i % 2 == 0) {
            finalColor = mix(finalColor, vec4(0,0,0,1), scanline_alpha);
        }
    }
}

