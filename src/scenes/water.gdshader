shader_type spatial;

// Adapted from default water shader from GLSL Sandbox.

#define TAU 6.28318530718
#define MAX_ITER 5

void fragment() {
    float t = TIME * 0.5 + 23.0;
    vec2 uv = FRAGCOORD.xy / vec2(1000, 1000);
    vec2 p = mod(uv * TAU, TAU) - 250.0;
    vec2 i = p;
    float c = 1.0;
    float inten = 0.005;
    for (int n = 0; n < 5; n++) {
        float tt = t * (1.0 - (3.5 / float(n + 1)));
        i = p + vec2(cos(tt - i.x) + sin(tt + i.y), sin(tt - i.y) + cos(tt + i.x));
        c += 1.0 / length(vec2(p.x / (sin(i.x + tt) / inten), p.y / (cos(i.y + tt) / inten)));
    }
    c /= float(MAX_ITER);
    c = 1.17 - pow(c, 1.4);
    vec3 color = vec3(pow(abs(c), 8.0));
    ALBEDO = clamp(color + vec3(0.0, 0.35, 0.5), 0.0, 1.0);
	ALPHA = 0.02;
}
