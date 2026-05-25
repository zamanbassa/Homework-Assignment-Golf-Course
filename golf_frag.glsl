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
uniform int       uUseTex;
uniform vec2      uTexOffset;
uniform float uTime;
uniform vec3  uLightDir;
uniform vec3  uLightColor;
uniform float uAmbient;
uniform vec3  uCamPos;
uniform float uTimeOfDay;
uniform int   uLampCount;
uniform vec3  uLampPos[48];
uniform vec3  uLampColor;
uniform int   uLampsOn;
uniform int   uSpotOn;
uniform vec3  uSpotPos;
uniform vec3  uSpotDir;
uniform float uSpotCutoff;
uniform float uSpotOuter;

uniform sampler2D uShadowMap;
uniform mat4      uLightSpace;
uniform int       uShadowOn;

out vec4 FragColor;

// PCF shadow
float computeShadow(){
    vec4 lsPos = uLightSpace * vec4(vFragPos, 1.0);
    vec3 proj  = lsPos.xyz / lsPos.w * 0.5 + 0.5;
    if(proj.x<0.0||proj.x>1.0||proj.y<0.0||proj.y>1.0||proj.z>1.0) return 0.0;
    float NdotL = max(dot(normalize(vNormal), normalize(uLightDir)), 0.0);
    float bias  = mix(0.003, 0.0003, NdotL);
    float shadow = 0.0;
    vec2 sz = 1.0 / vec2(textureSize(uShadowMap, 0));
    for(int x=-1;x<=1;x++)
        for(int y=-1;y<=1;y++){
            float d = texture(uShadowMap, proj.xy + vec2(x,y)*sz).r;
            shadow += (proj.z - bias > d) ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}

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
        // grass
        float n = fbm(vUV * 12.0);
        float stripe = 0.5 + 0.5*sin(vUV.x * 40.0 + vUV.y * 5.0);
        col = vec3(0.12, 0.45+0.08*stripe, 0.10) * (0.85 + 0.15*n);
    } else if (uSurface == 1){
        // tee
        float n = fbm(vUV * 25.0);
        col = vec3(0.18, 0.62, 0.15) * (0.90 + 0.10*n);
    } else if (uSurface == 2){
        // rough
        float n = fbm(vUV * 8.0);
        col = vec3(0.10, 0.32+0.10*n, 0.08);
    } else if (uSurface == 3){
        // path
        vec2 cell = fract(vUV * 3.0);
        float grout = step(cell.x,0.05)+step(1.0-cell.x,0.05)+step(cell.y,0.05)+step(1.0-cell.y,0.05);
        float shade = 0.55 + 0.15*hash(floor(vUV*3.0));
        col = mix(vec3(shade,shade*0.95,shade*0.88), vec3(0.30,0.30,0.30), clamp(grout,0.0,1.0));
    } else if (uSurface == 4){
        // sand
        float n = fbm(vUV * 22.0);
        col = vec3(0.88, 0.78, 0.52) * (0.75 + 0.25*n);
    } else if (uSurface == 5){
        // water
        float dep = 0.55 + 0.15*sin(vUV.x*7.0+uTime);
        col = vec3(0.26+0.06*dep, 0.36+0.08*dep, 0.50+0.08*dep);
    } else if (uSurface == 6){
        // wood
        float grain = fbm(vec2(vUV.x*2.0, vUV.y*30.0));
        col = vec3(0.55+0.15*grain, 0.38+0.10*grain, 0.18+0.05*grain);
    } else if (uSurface == 7){
        // windmill body
        float n = fbm(vUV * 6.0);
        col = vec3(0.68+0.08*n, 0.66+0.08*n, 0.62+0.06*n);
    } else if (uSurface == 8){
        // concrete
        float n = fbm(vUV * 4.0);
        col = vec3(0.62+0.10*n, 0.62+0.10*n, 0.60+0.08*n);
    } else if (uSurface == 9){
        // terracotta
        vec2 cell = fract(vUV * 4.0);
        float grout = step(cell.x,0.06)+step(1.0-cell.x,0.06)+step(cell.y,0.06)+step(1.0-cell.y,0.06);
        float var = 0.85 + 0.15*hash(floor(vUV*4.0));
        col = mix(vec3(0.78*var, 0.42*var, 0.28*var), vec3(0.35,0.30,0.28), clamp(grout,0.0,1.0));
    } else if (uSurface == 10){
        // rock
        float n = fbm(vUV * 9.0);
        float coarse = hash(floor(vUV * 5.0));
        col = vec3(0.40+0.20*coarse+0.08*n, 0.38+0.18*coarse+0.06*n, 0.34+0.16*coarse+0.05*n);
    } else if (uSurface == 11){
        // trunk
        float ring = 0.5 + 0.5*sin(vUV.y * 60.0);
        float n = fbm(vUV * 8.0);
        col = vec3(0.45+0.10*ring, 0.28+0.08*ring, 0.12+0.05*ring) * (0.85+0.15*n);
    } else if (uSurface == 12){
        // palm leaf
        float n = fbm(vUV * 15.0);
        float vein = step(abs(vUV.x - 0.5), 0.03);
        col = mix(vec3(0.10, 0.52+0.12*n, 0.08), vec3(0.55, 0.65, 0.20), vein*0.3);
    } else if (uSurface == 13){
        // lamp post
        float n = fbm(vUV * 12.0) * 0.1;
        col = vec3(0.20+n, 0.20+n, 0.22+n);
    } else if (uSurface == 14){
        // lamp glow
        col = uLampsOn == 1 ? vec3(1.0, 0.92, 0.65) : vec3(0.25, 0.25, 0.22);
    } else if (uSurface == 15){
        // ball
        col = vec3(0.96, 0.96, 0.96);
    } else if (uSurface == 16){
        // flag pole
        col = vec3(0.85, 0.85, 0.85);
    } else if (uSurface == 17){
        // flag
        float wave = 0.5 + 0.5*sin(vUV.x * 12.0 + uTime * 3.0);
        col = vec3(0.90, 0.10+0.05*wave, 0.10);
    } else if (uSurface == 18){
        // windmill blade
        float edge = step(0.9, vUV.x) + step(0.9, vUV.y) + step(vUV.x, 0.1) + step(vUV.y, 0.1);
        col = mix(vec3(0.94, 0.94, 0.94), vec3(0.65, 0.65, 0.68), clamp(edge, 0.0, 1.0));
    } else if (uSurface == 19){
        // cup
        col = vec3(0.05, 0.05, 0.05);
    } else if (uSurface == 20){
        // roof tile
        vec2 tUV  = vUV * vec2(10.0, 8.0);
        float row = floor(tUV.y);
        vec2  tF  = fract(tUV + vec2(mod(row, 2.0) * 0.5, 0.0));
        float grout = step(0.05, tF.x) * step(0.05, tF.y);
        float baseN = fbm(vUV * 6.0) * 0.05;
        col = mix(vec3(0.10, 0.10, 0.12), vec3(0.21+baseN, 0.22+baseN, 0.25+baseN), grout);
    } else if (uSurface == 21){
        // red brick
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
        // hedge
        float n = fbm(vUV * 12.0);
        float tip = 0.5 + 0.5*sin(vUV.x * 18.0 + vUV.y * 7.0);
        col = vec3(0.05+0.03*n, 0.24+0.10*n+0.04*tip, 0.05+0.03*n);
    } else if (uSurface == 23){
        // soil
        float n = fbm(vUV * 7.0);
        col = vec3(0.38+0.12*n, 0.28+0.07*n, 0.17+0.05*n);
    } else if (uSurface == 24){
        // drone body
        float n = fbm(vUV * 8.0) * 0.04;
        col = vec3(0.25+n, 0.25+n, 0.28+n);
    } else if (uSurface == 25){
        // propeller
        float n = fbm(vUV * 6.0) * 0.03;
        col = vec3(0.16+n, 0.16+n, 0.18+n);
    } else if (uSurface == 26){
        // dark earth
        float n = fbm(vUV * 10.0);
        float coarse = hash(floor(vUV * 5.0));
        col = vec3(0.22+0.08*coarse+0.05*n, 0.14+0.05*coarse+0.03*n, 0.06+0.03*coarse+0.02*n);
    } else if (uSurface == 27){
        // windmill base
        float n = fbm(vUV * 6.0);
        col = vec3(0.28+0.06*n, 0.15+0.04*n, 0.06+0.02*n);
    } else if (uSurface == 28){
        // step 1
        float n = fbm(vUV * 7.0);
        col = vec3(0.40+0.08*n, 0.22+0.05*n, 0.09+0.03*n);
    } else if (uSurface == 29){
        // step 2-3
        float n = fbm(vUV * 7.0);
        col = vec3(0.52+0.09*n, 0.32+0.06*n, 0.14+0.04*n);
    } else if (uSurface == 30){
        // step 4
        float n = fbm(vUV * 8.0);
        col = vec3(0.65+0.10*n, 0.44+0.07*n, 0.22+0.05*n);
    } else if (uSurface == 31){
        // glass
        float n = fbm(vUV * 4.0);
        col = vec3(0.22+0.04*n, 0.32+0.07*n, 0.48+0.10*n);
    } else if (uSurface == 32){
        // door
        float grain = fbm(vec2(vUV.x*1.5, vUV.y*25.0));
        col = vec3(0.42+0.10*grain, 0.26+0.06*grain, 0.12+0.03*grain);
    } else if (uSurface == 33){
        // frame
        float n = fbm(vUV * 8.0) * 0.02;
        col = vec3(0.17+n, 0.18+n, 0.20+n);
    } else if (uSurface == 34){
        // sign
        float n = fbm(vUV * 3.0) * 0.04;
        col = vec3(0.92+n, 0.88+n, 0.78+n);
    } else {
        col = vec3(1.0, 0.0, 1.0); // missing
    }

    return col;
}

void main(){
    vec3 norm = normalize(vNormal);

    // water ripple
    if (uSurface == 5){
        float wx = sin(vUV.x*18.0 + uTime*2.2) * 0.07;
        float wz = sin(vUV.y*13.0 + uTime*1.7) * 0.06;
        float w2 = cos((vUV.x+vUV.y)*9.0 + uTime*1.5) * 0.04;
        norm = normalize(norm + vec3(wx, 0.0, wz+w2));
    }

    vec3 col  = calcSurface();

    // texture
    if (uUseTex == 1){
        vec4 t = texture(uTex, vUV + uTexOffset);
        if (t.a < 0.15) discard;
        col = t.rgb;
    }

    // emissive
    if (uSurface == 14){
        FragColor = vec4(col, 1.0);
        return;
    }

    // sun light
    float rawDot  = dot(norm, normalize(uLightDir));
    float sunDiff = (uSurface == 12) ? abs(rawDot) : max(rawDot, 0.0);
    float sunFactor = 0.0;
    if (uTimeOfDay >= 0.25 && uTimeOfDay <= 0.75)
        sunFactor = sin((uTimeOfDay - 0.25) / 0.5 * 3.14159);
    sunFactor = clamp(sunFactor, 0.0, 1.0);

    float shadowFactor = (uShadowOn == 1) ? computeShadow() : 0.0;
    float sunMult = sunFactor * (1.0 - shadowFactor * 0.75);

    vec3 lighting = col * uAmbient;
    lighting += col * sunDiff * sunMult * uLightColor;

    // specular
    if (uSurface == 5 || uSurface == 15){
        vec3 viewDir  = normalize(uCamPos - vFragPos);
        vec3 reflDir  = reflect(-normalize(uLightDir), norm);
        float specPow = (uSurface == 5) ? 64.0 : 32.0;
        float spec = pow(max(dot(viewDir, reflDir), 0.0), specPow);
        lighting += vec3(spec) * sunMult * 0.6;
    }

    // lamps
    if (uLampsOn == 1){
        for (int i = 0; i < uLampCount; i++){
            vec3 ldir = uLampPos[i] - vFragPos;
            float dist = length(ldir);
            ldir = normalize(ldir);
            float diff = (uSurface == 12) ? abs(dot(norm, ldir)) : max(dot(norm, ldir), 0.0);
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
