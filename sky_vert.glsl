#version 330 core
<<<<<<< HEAD
layout(location = 0) in vec3 aPos;

uniform mat4 uVP;

out vec3 vDir;

void main(){
    vDir = aPos;
    vec4 p = uVP * vec4(aPos, 1.0);
    gl_Position = p.xyww;
}
=======

layout(location = 0) in vec3 aPos;

out vec3 vDir;

uniform mat4 uVP;

void main(){

    vDir = aPos;

    vec4 pos = uVP * vec4(aPos * 500.0, 1.0);

    gl_Position = pos.xyww;
}
>>>>>>> 1ec622464d8af3aea218c363d4bd521a3e6a13b7
