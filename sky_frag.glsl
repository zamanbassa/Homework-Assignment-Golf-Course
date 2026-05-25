#version 330 core

in vec3 vDir;
out vec4 FragColor;

uniform float uTimeOfDay;
uniform sampler2D skyTex;

vec3 nightColor(vec3 dir){

    float star = step(
        0.998,
        fract(
            sin(dot(floor(dir * 200.0),
            vec3(127.1, 311.7, 74.3))) * 43758.5453
        )
    );

    vec3 sky = vec3(0.02, 0.03, 0.08);

    return sky + vec3(star) * 0.8;
}

vec3 dawnColor(vec3 dir){

    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);

    vec3 horizon = vec3(1.0, 0.45, 0.15);
    vec3 zenith  = vec3(0.35, 0.55, 0.95);
    vec3 ground = mix(horizon, zenith, 0.15);

    if (dir.y < 0.0)
        return mix(
            ground,
            horizon,
            clamp(-dir.y * 4.0, 0.0, 1.0)
        );

    return mix(horizon, zenith, h * h);
}

vec3 duskColor(vec3 dir){

    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);

    vec3 horizon = vec3(1.0, 0.35, 0.10);
    vec3 zenith  = vec3(0.20, 0.22, 0.60);
    vec3 ground = mix(horizon, zenith, 0.15);

    if (dir.y < 0.0)
        return mix(
            ground,
            horizon,
            clamp(-dir.y * 4.0, 0.0, 1.0)
        );

    return mix(horizon, zenith, h * h);
}

vec3 daySky(vec3 dir){

    float h = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);

	vec3 horizon = vec3(0.55, 0.75, 0.95);
    vec3 zenith  = vec3(0.20, 0.50, 0.95);
    vec3 ground  = vec3(0.40, 0.55, 0.70);

    vec3 baseSky;
    if (dir.y < 0.0)
        baseSky = mix(
            ground,
            horizon,
            clamp(-dir.y * 6.0, 0.0, 1.0)
        );
    else
        baseSky = mix(horizon, zenith, pow(h, 1.8));

    float u = atan(dir.z, dir.x) / (2.0 * 3.14159265) + 0.5;
    float v = dir.y * 0.55 + 0.45;

    vec3 tex = texture(
        skyTex,
        vec2(u, 1.0 - v)
    ).rgb;

    float cloudMask = dot(tex, vec3(0.333));
	float horizonFade = smoothstep(0.0, 0.15, dir.y);
	cloudMask = smoothstep(0.75, 0.98, cloudMask) * horizonFade;

    vec3 finalSky = mix(
        baseSky,
        vec3(1.0),
        cloudMask * 0.35
    );

    return finalSky;
}

void main(){

    vec3 dir = normalize(vDir);

    vec3 col;

    float t = uTimeOfDay;

    // Night
    if (t < 0.15){

        col = nightColor(dir);

    }

    // dawn
    else if (t < 0.30){

        float f = (t - 0.15) / 0.15;

        col = mix(
            nightColor(dir),
            dawnColor(dir),
            f
        );

    }

    // day
    else if (t < 0.40){

        float f = (t - 0.30) / 0.10;

        col = mix(
            dawnColor(dir),
            daySky(dir),
            f
        );

    }

    // full day
    else if (t < 0.65){

        col = daySky(dir);

    }

    // dusk
    else if (t < 0.75){

        float f = (t - 0.65) / 0.10;

        col = mix(
            daySky(dir),
            duskColor(dir),
            f
        );

    }

    // night
    else if (t < 0.85){

        float f = (t - 0.75) / 0.10;

        col = mix(
            duskColor(dir),
            nightColor(dir),
            f
        );

    }

    // Night
    else {

        col = nightColor(dir);

    }

    FragColor = vec4(col, 1.0);
}
