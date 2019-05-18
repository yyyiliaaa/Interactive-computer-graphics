/* 
File Name: "vfirework.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in vec3 fireworkVelocity;
in vec3 fireworkColors;

out vec4 pointcolor;
out vec4 pointpos;

uniform float t;
uniform mat4 ModelView;
uniform mat4 Projection;

void main(){
    float a = 0.00000049;
    //vec4 pointpos;
    
    
    pointpos.x = 0 + 0.001 * fireworkVelocity.x * t;
    pointpos.y = 0.1 + 0.001 * fireworkVelocity.y * t - 0.5 * a * t * t;
    pointpos.z = 0 + 0.001 * fireworkVelocity.z * t;
    pointpos.w = 1;
    
    
    gl_Position = Projection * ModelView * pointpos;
    pointcolor = vec4(fireworkColors.xyz, 1);
}
