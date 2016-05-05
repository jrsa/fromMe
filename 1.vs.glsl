# version 410 core

in vec2 position;
in vec2 velocity;
in vec2 originalPos;

out vec2 outPosition;
out vec2 outVelocity;

uniform float time;
uniform vec2 mousePos;
uniform usampler2D depth;

void main() {
    vec2 newVelocity = originalPos - position;
    vec2 tc = -originalPos;

    tc *= mat2(0.5, 0.0, 0.0, 0.5);
    tc += vec2(0.5); 

    uvec4 depth_pixel = texture(depth, tc);
    float d = normalize(depth_pixel.r);

    //if (length(mousePos - originalPos) < 0.75f) {
    //    vec2 acceleration = 1.5f * normalize(mousePos - position);
    //    newVelocity = velocity + acceleration * time;
    //}
    if (d < 0.5) {
        vec2 acceleration = 2.0 * normalize(position);
        newVelocity = velocity + acceleration * time;
    }

    if (length(newVelocity) > 1.0f) {
        newVelocity = normalize(newVelocity);
    }

    vec2 newPosition = position + newVelocity * time;

    outPosition = newPosition;
    outVelocity = newVelocity;
    gl_Position = vec4(newPosition, 0.0, 1.0);

    // debug only
    //outPosition = originalPos;
    //outVelocity = vec2(d, 0.0);
    //gl_Position = vec4(originalPos, 0.0, 1.0);
}