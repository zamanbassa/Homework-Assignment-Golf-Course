#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
uniform vec4      uColor;
uniform int       uUseTex;
out vec4 FragColor;
void main(){
    if(uUseTex == 1){
        float a = texture(uTex, vUV).r;
        if(a < 0.05) discard;
        FragColor = vec4(uColor.rgb, uColor.a * a);
    } else {
        FragColor = uColor;
    }
}
