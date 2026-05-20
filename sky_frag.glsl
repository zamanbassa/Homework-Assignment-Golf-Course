#version 330 core
in  vec3 vDir;
out vec4 FragColor;

uniform float uTimeOfDay; // 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1=midnight

vec3 nightColor(vec3 dir){
    // Very dark blue sky with faint stars
    float star = step(0.998, fract(sin(dot(floor(dir * 200.0), vec3(127.1, 311.7, 74.3))) * 43758.5453));
    vec3 sky = vec3(0.02, 0.03, 0.08);
    return sky + vec3(star) * 0.8;
}

vec3 dawnColor(vec3 dir){
    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 horizon = vec3(1.0, 0.45, 0.15);
    vec3 zenith  = vec3(0.35, 0.55, 0.95);
    vec3 ground  = vec3(0.55, 0.40, 0.25);
    if (dir.y < 0.0) return mix(ground, horizon, clamp(-dir.y * 4.0, 0.0, 1.0));
    return mix(horizon, zenith, h * h);
}

vec3 noonColor(vec3 dir){
    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 horizon = vec3(0.72, 0.88, 1.0);
    vec3 zenith  = vec3(0.20, 0.50, 0.95);
    vec3 ground  = vec3(0.45, 0.60, 0.30);
    if (dir.y < 0.0) return mix(ground, horizon, clamp(-dir.y * 4.0, 0.0, 1.0));
    return mix(horizon, zenith, h);
}

vec3 duskColor(vec3 dir){
    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 horizon = vec3(1.0, 0.35, 0.10);
    vec3 zenith  = vec3(0.20, 0.22, 0.60);
    vec3 ground  = vec3(0.40, 0.30, 0.25);
    if (dir.y < 0.0) return mix(ground, horizon, clamp(-dir.y * 4.0, 0.0, 1.0));
    return mix(horizon, zenith, h * h);
}

void main(){
    vec3 dir = normalize(vDir);
    vec3 col;

    // t in [0,1], transitions: night->dawn->noon->dusk->night
    float t = uTimeOfDay;

    if (t < 0.15){
        // Night (0 → 0.15)
        float f = t / 0.15;
        col = mix(nightColor(dir), nightColor(dir), f); // pure night
    } else if (t < 0.30){
        // Night -> Dawn  (0.15 → 0.30)
        float f = (t - 0.15) / 0.15;
        col = mix(nightColor(dir), dawnColor(dir), f);
    } else if (t < 0.40){
        // Dawn -> Noon  (0.30 → 0.40)
        float f = (t - 0.30) / 0.10;
        col = mix(dawnColor(dir), noonColor(dir), f);
    } else if (t < 0.65){
        // Noon  (0.40 → 0.65)
        col = noonColor(dir);
    } else if (t < 0.75){
        // Noon -> Dusk  (0.65 → 0.75)
        float f = (t - 0.65) / 0.10;
        col = mix(noonColor(dir), duskColor(dir), f);
    } else if (t < 0.85){
        // Dusk -> Night (0.75 → 0.85)
        float f = (t - 0.75) / 0.10;
        col = mix(duskColor(dir), nightColor(dir), f);
    } else {
        // Night (0.85 → 1)
        col = nightColor(dir);
    }

    FragColor = vec4(col, 1.0);
}
