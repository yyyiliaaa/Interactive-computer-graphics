/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color;
in  float distance;
in  vec2 groundtexCoord;
in  vec2 spheretexCoord;
in  vec2 latticetexCoord;

out vec4 fColor;

uniform int texturepart;    //0:others; 1:floor; 2:sphere; 3:shadow
uniform int fogoption;
uniform int texground_flag; // 0: no texture application; 1: (obj color) * (texture color)
uniform int texsphere_flag;
uniform int latticeoption;

uniform vec4 fogcolor;
uniform sampler2D checkerboard;
uniform sampler1D stripe;

void main() 
{
    float density = 0.09;
    float f;
    vec4 mcolor = color;

    if(texturepart == 0){
        mcolor = color;
    }
    //floor
    else if(texturepart == 1){
        if(texground_flag == 0)
            mcolor = color;
        else if(texground_flag == 1)
            mcolor = color * texture( checkerboard, groundtexCoord );
    }
    //sphere
    else if(texturepart == 2){
        if(texsphere_flag == 0)
            mcolor = color;
        else if(texsphere_flag == 1){
            mcolor = color * texture(stripe, spheretexCoord[0]);
        }
        else{
            mcolor = color * texture(checkerboard, spheretexCoord);
            if(mcolor[0] == 0)
                mcolor = color * vec4(0.9, 0.1, 0.1, 1.0);
        }
        if(latticeoption == 0){}
        else{
            if(fract(4*latticetexCoord[0]) < 0.35 && fract(4*latticetexCoord[1]) < 0.35){
                discard;
            }
        }
    }
    else{
        if(latticeoption == 0){}
        else{
            if(fract(4*latticetexCoord[0]) < 0.35 && fract(4*latticetexCoord[1]) < 0.35){
                discard;
            }
        }
    }
    
    
    //fog effect
    if(fogoption == 0){
        fColor = mcolor;
    }
    else if(fogoption == 1){
        f = (18.0-distance)/(18.0-0.0);
        f = clamp(f,0,1);
        fColor.xyz = clamp(mix(fogcolor.xyz, mcolor.xyz, f), 0.0, 1.0);
        fColor.w = mcolor.w;
    }
    else if(fogoption == 2){
        f = exp(-density * distance);
        fColor.xyz = clamp(mix(fogcolor.xyz, mcolor.xyz, f), 0.0, 1.0);
        fColor.w = mcolor.w;
    }
    else{
        f = exp(-pow(density * distance, 2));
        fColor.xyz = clamp(mix(fogcolor.xyz, mcolor.xyz, f), 0.0, 1.0);
        fColor.w = mcolor.w;
    }
}

