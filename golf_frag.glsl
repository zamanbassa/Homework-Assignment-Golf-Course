#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vUV;

// Surface types
// 0 = fairway/green grass
// 1 = tee box (lighter grass)
// 2 = rough (dark grass)
// 3 = path/walkway (stone)
// 4 = sand bunker
// 5 = water hazard
// 6 = wood (obstacles, bridges)
// 7 = metal (windmill body)
// 8 = concrete (walls, borders)
// 9 = restaurant floor
// 10 = rock/boulder
// 11 = palm tree trunk
// 12 = palm tree leaf
// 13 = lamp post metal
// 14 = lamp glow (emissive)
// 15 = ball (white)
// 16 = flag pole
// 17 = flag fabric (red)
// 18 = windmill blade (white/grey)
// 19 = cup/hole rim (black)
// 20 = restaurant roof (terracotta)

uniform int       uSurface;
uniform sampler2D uTex;
uniform int       uUseTex;  // 1 = sample uTex and override calcSurface colour
uniform float uTime;
uniform vec3  uLightDir;      // sun direction (world-space)
uniform vec3  uLightColor;    // sun color
uniform float uAmbient;       // ambient intensity
uniform vec3  uCamPos;
uniform float uTimeOfDay;     // 0-1
uniform int   uLampCount;
uniform vec3  uLampPos[32];
uniform vec3  uLampColor;
uniform int   uLampsOn;       // 1 if night
uniform int   uSpotOn;        // 1 = spotlight active
uniform vec3  uSpotPos;       // drone world position
uniform vec3  uSpotDir;       // normalised direction
uniform float uSpotCutoff;    // cos of cone half-angle
uniform float uSpotOuter;     // cos of outer cone (soft edge)

out vec4 FragColor;

float hash(vec2 p){
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}
float hash3(vec3 p){
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.3))) * 43758.5453);
}
float vnoise(vec2 p){
    vec2 i = floor(p), f = fract(p);
    f = f*f*(3.0-2.0*f);
    return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),
               mix(hash(i+vec2(0,1)),hash(i+vec2(1,1)),f.x),f.y);
}
float fbm(vec2 p){
    return 0.5*vnoise(p) + 0.25*vnoise(p*2.1) + 0.125*vnoise(p*4.3);
}

vec3 calcSurface(){
    vec3 col = vec3(1.0);

    if (uSurface == 0){
        // Fairway/green — medium green
        float n = fbm(vUV * 12.0);
        float stripe = 0.5 + 0.5*sin(vUV.x * 40.0 + vUV.y * 5.0);
        col = vec3(0.12, 0.45+0.08*stripe, 0.10) * (0.85 + 0.15*n);
    } else if (uSurface == 1){
        // Tee box — bright manicured green
        float n = fbm(vUV * 25.0);
        col = vec3(0.18, 0.62, 0.15) * (0.90 + 0.10*n);
    } else if (uSurface == 2){
        // Rough — darker patchy grass
        float n = fbm(vUV * 8.0);
        col = vec3(0.10, 0.32+0.10*n, 0.08);
    } else if (uSurface == 3){
        // Path — stone/concrete walkway
        vec2 cell = fract(vUV * 3.0);
        float grout = step(cell.x,0.05)+step(1.0-cell.x,0.05)+step(cell.y,0.05)+step(1.0-cell.y,0.05);
        float shade = 0.55 + 0.15*hash(floor(vUV*3.0));
        col = mix(vec3(shade,shade*0.95,shade*0.88), vec3(0.30,0.30,0.30), clamp(grout,0.0,1.0));
    } else if (uSurface == 4){
        // Sand bunker
        float n = fbm(vUV * 22.0);
        col = vec3(0.88, 0.78, 0.52) * (0.75 + 0.25*n);
    } else if (uSurface == 5){
        // Water — animated, muted grey-blue
        float dep = 0.55 + 0.15*sin(vUV.x*7.0+uTime);
        col = vec3(0.26+0.06*dep, 0.36+0.08*dep, 0.50+0.08*dep);
    } else if (uSurface == 6){
        // Wood (bridges/obstacles)
        float grain = fbm(vec2(vUV.x*2.0, vUV.y*30.0));
        col = vec3(0.55+0.15*grain, 0.38+0.10*grain, 0.18+0.05*grain);
    } else if (uSurface == 7){
        // Windmill body — smooth grey stone cylinder
        float n = fbm(vUV * 6.0);
        col = vec3(0.68+0.08*n, 0.66+0.08*n, 0.62+0.06*n);
    } else if (uSurface == 8){
        // Concrete walls/borders
        float n = fbm(vUV * 4.0);
        col = vec3(0.62+0.10*n, 0.62+0.10*n, 0.60+0.08*n);
    } else if (uSurface == 9){
        // Restaurant floor — terracotta tiles
        vec2 cell = fract(vUV * 4.0);
        float grout = step(cell.x,0.06)+step(1.0-cell.x,0.06)+step(cell.y,0.06)+step(1.0-cell.y,0.06);
        float var = 0.85 + 0.15*hash(floor(vUV*4.0));
        col = mix(vec3(0.78*var, 0.42*var, 0.28*var), vec3(0.35,0.30,0.28), clamp(grout,0.0,1.0));
    } else if (uSurface == 10){
        // Rock/boulder
        float n = fbm(vUV * 9.0);
        float coarse = hash(floor(vUV * 5.0));
        col = vec3(0.40+0.20*coarse+0.08*n, 0.38+0.18*coarse+0.06*n, 0.34+0.16*coarse+0.05*n);
    } else if (uSurface == 11){
        // Palm tree trunk — brown with ring texture
        float ring = 0.5 + 0.5*sin(vUV.y * 60.0);
        float n = fbm(vUV * 8.0);
        col = vec3(0.45+0.10*ring, 0.28+0.08*ring, 0.12+0.05*ring) * (0.85+0.15*n);
    } else if (uSurface == 12){
        // Palm leaf — bright tropical green
        float n = fbm(vUV * 15.0);
        float vein = step(abs(vUV.x - 0.5), 0.03);
        col = mix(vec3(0.10, 0.52+0.12*n, 0.08), vec3(0.55, 0.65, 0.20), vein*0.3);
    } else if (uSurface == 13){
        // Lamp post — dark metal
        float n = fbm(vUV * 12.0) * 0.1;
        col = vec3(0.20+n, 0.20+n, 0.22+n);
    } else if (uSurface == 14){
        // Lamp glow — emissive yellow-white
        col = uLampsOn == 1 ? vec3(1.0, 0.92, 0.65) : vec3(0.25, 0.25, 0.22);
    } else if (uSurface == 15){
        // Golf ball — white
        col = vec3(0.96, 0.96, 0.96);
    } else if (uSurface == 16){
        // Flag pole — white metal
        col = vec3(0.85, 0.85, 0.85);
    } else if (uSurface == 17){
        // Flag — bright red
        float wave = 0.5 + 0.5*sin(vUV.x * 12.0 + uTime * 3.0);
        col = vec3(0.90, 0.10+0.05*wave, 0.10);
    } else if (uSurface == 18){
        // Windmill blade — white with grey edge
        float edge = step(0.9, vUV.x) + step(0.9, vUV.y) + step(vUV.x, 0.1) + step(vUV.y, 0.1);
        col = mix(vec3(0.94, 0.94, 0.94), vec3(0.65, 0.65, 0.68), clamp(edge, 0.0, 1.0));
    } else if (uSurface == 19){
        // Cup/hole — black
        col = vec3(0.05, 0.05, 0.05);
    } else if (uSurface == 20){
        // Restaurant roof — terracotta/clay
        float n = fbm(vUV * 5.0);
        col = vec3(0.72+0.08*n, 0.32+0.06*n, 0.18+0.04*n);
    } else if (uSurface == 21){
        // Red brick path — staggered bond pattern
        float row  = floor(vUV.y * 8.0);
        float off  = mod(row, 2.0) * 0.5;
        vec2  bUV  = vec2(fract(vUV.x * 4.0 + off), fract(vUV.y * 8.0));
        float mort = step(bUV.x, 0.06) + step(1.0-bUV.x, 0.06)
                   + step(bUV.y, 0.09) + step(1.0-bUV.y, 0.09);
        float var  = 0.78 + 0.14*hash(floor(vec2(vUV.x*4.0+off, vUV.y*8.0)));
        col = mix(vec3(0.70*var, 0.26*var, 0.16*var),
                  vec3(0.44, 0.42, 0.40),
                  clamp(mort, 0.0, 1.0));
    } else if (uSurface == 22){
        // Hedge / boundary bush
        float n = fbm(vUV * 12.0);
        float tip = 0.5 + 0.5*sin(vUV.x * 18.0 + vUV.y * 7.0);
        col = vec3(0.05+0.03*n, 0.24+0.10*n+0.04*tip, 0.05+0.03*n);
    } else if (uSurface == 23){
        // Soil / dirt base
        float n = fbm(vUV * 7.0);
        col = vec3(0.38+0.12*n, 0.28+0.07*n, 0.17+0.05*n);
    } else if (uSurface == 24){
        // Drone body — charcoal grey
        float n = fbm(vUV * 8.0) * 0.04;
        col = vec3(0.25+n, 0.25+n, 0.28+n);
    } else if (uSurface == 25){
        // Drone propellers — dark grey
        float n = fbm(vUV * 6.0) * 0.03;
        col = vec3(0.16+n, 0.16+n, 0.18+n);
    } else {
        col = vec3(1.0, 0.0, 1.0); // missing surface — magenta
    }

    return col;
}

void main(){
    vec3 norm = normalize(vNormal);

    // Water: animated normal perturbation for ripple effect
    if (uSurface == 5){
        float wx = sin(vUV.x*18.0 + uTime*2.2) * 0.07;
        float wz = sin(vUV.y*13.0 + uTime*1.7) * 0.06;
        float w2 = cos((vUV.x+vUV.y)*9.0 + uTime*1.5) * 0.04;
        norm = normalize(norm + vec3(wx, 0.0, wz+w2));
    }

    vec3 col  = calcSurface();

    // Texture override (rock BMP or plant PNG with alpha cutout)
    if (uUseTex == 1){
        vec4 t = texture(uTex, vUV);
        if (t.a < 0.15) discard;
        col = t.rgb;
    }

    // Emissive — skip lighting
    if (uSurface == 14){
        FragColor = vec4(col, 1.0);
        return;
    }

    // Sun light
    float sunDiff = max(dot(norm, normalize(uLightDir)), 0.0);
    // smooth sun based on time
    float sunFactor = 0.0;
    if (uTimeOfDay >= 0.25 && uTimeOfDay <= 0.75)
        sunFactor = sin((uTimeOfDay - 0.25) / 0.5 * 3.14159);
    sunFactor = clamp(sunFactor, 0.0, 1.0);

    vec3 lighting = col * uAmbient;
    lighting += col * sunDiff * sunFactor * uLightColor;

    // Specular (for water, ball)
    if (uSurface == 5 || uSurface == 15){
        vec3 viewDir  = normalize(uCamPos - vFragPos);
        vec3 reflDir  = reflect(-normalize(uLightDir), norm);
        float specPow = (uSurface == 5) ? 64.0 : 32.0;
        float spec = pow(max(dot(viewDir, reflDir), 0.0), specPow);
        lighting += vec3(spec) * sunFactor * 0.6;
    }

    // Lamp posts (point lights at night)
    if (uLampsOn == 1){
        for (int i = 0; i < uLampCount; i++){
            vec3 ldir = uLampPos[i] - vFragPos;
            float dist = length(ldir);
            ldir = normalize(ldir);
            float diff = max(dot(norm, ldir), 0.0);
            float att  = 1.0 / (1.0 + 0.18*dist + 0.06*dist*dist);
            lighting += col * diff * att * uLampColor;
        }
    }

	if (uSpotOn == 1){
        vec3  toFrag  = vFragPos - uSpotPos;
        vec3  sdir    = normalize(uSpotDir);
        float cosA    = dot(normalize(toFrag), sdir);
        float dist    = length(toFrag);
        float att     = 1.0 / (1.0 + 0.05*dist + 0.01*dist*dist);
        float cone    = smoothstep(uSpotOuter, uSpotCutoff, cosA);
        float diff    = max(dot(norm, -sdir), 0.0);
        lighting += col * diff * att * cone * vec3(1.0, 0.97, 0.88) * 2.5;
    }

    FragColor = vec4(lighting, 1.0);
}
