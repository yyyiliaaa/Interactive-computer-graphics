/* 
File Name: "ffirework.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version
in  vec4 pointcolor;
in  vec4 pointpos;

out vec4 fColor;

void main(){
    if(pointpos.y < 0.1){
        discard;
    }
    fColor = pointcolor;
}
