precision mediump float;
precision mediump int;

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec3 flash_color; // What color do you want to flash it?
uniform float time;

out vec4 finalColor;

void main() {
    bool flash = sin(time*3.1415*2.0*50.0) > 0.0;

    vec4 texel = texture(texture0, fragTexCoord);

    if (flash) {
        finalColor = vec4(flash_color, texel.a);
    } else {
        finalColor = texel;
    }
}
