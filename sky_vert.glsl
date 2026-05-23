#version 330 core

layout(location = 0) in vec3 aPos;

out vec3 vDir;

uniform mat4 uVP;

void main(){

    vDir = aPos;

    vec4 pos = uVP * vec4(aPos * 500.0, 1.0);

    gl_Position = pos.xyww;
}