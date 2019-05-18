/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
in  vec4 vColor;
in  vec2 vTexCoord;

out vec4 color;
out vec2 groundtexCoord;
out vec2 spheretexCoord;
out vec2 latticetexCoord;
out float distance;

uniform  int spaceoption;
uniform  int texsphere_flag;
uniform  int texsphere_dir;  //0:vertical; 1:slanted
uniform  int latticeoption; //0: no 1: yes
uniform  int latticedir; //0:upright; 1:tilted
uniform  int colorFlag;
uniform  int lightoption;
uniform vec4 gAmbientProduct;
uniform vec4 dAmbientProduct, dDiffuseProduct, dSpecularProduct;
uniform vec4 sAmbientProduct, sDiffuseProduct, sSpecularProduct;
uniform vec3 Lightfrom;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix;
uniform vec4 LightPosition;   // Must be in Eye Frame
uniform vec4 towardpoint;
uniform float Shininess;
uniform float angle;
uniform float exponent;

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation

void main()
{
    
    gl_Position = Projection * ModelView * vPosition;
    groundtexCoord = vTexCoord;
    
    vec3 pos = (ModelView * vPosition).xyz;
    vec3 postex;
    distance = length(pos);
    
    if(spaceoption == 0){
        postex = vPosition.xyz;
    }
    else{
        postex = pos;
    }
    if(texsphere_flag == 0){}
    else if(texsphere_flag == 1){
        //line
        if(texsphere_dir == 0)
            spheretexCoord[0] = 2.5*postex[0];
        else
            spheretexCoord[0] = 1.5*(postex[0]+postex[1]+postex[2]);
    }
    else if(texsphere_flag == 2){
        if(texsphere_dir == 0){
            spheretexCoord[0] = 0.5 * (postex[0] + 1);
            spheretexCoord[1] = 0.5 * (postex[1] + 1);
        }
        else{
            spheretexCoord[0] = 0.3 * (postex[0] + postex[1] + postex[2]);
            spheretexCoord[1] = 0.3 * (postex[0] - postex[1] + postex[2]);
        }
    }
    if(latticeoption == 0){}
    else{
        if(latticedir == 0){
            //upright
            latticetexCoord[0] = 0.5 * (postex[0] + 1);
            latticetexCoord[1] = 0.5 * (postex[1] + 1);
        }
        else{
            //titled
            latticetexCoord[0] = 0.3 * (postex[0] + postex[1] + postex[2]);
            latticetexCoord[1] = 0.3 * (postex[0] - postex[1] + postex[2]);
        }
    }
    
    if(colorFlag == 1){
        color = vColor;
    }
    if(colorFlag == 0){
        vec4 ambient;
        float d;
        vec4  diffuse;
        float s;
        vec4  specular;
        vec3 E = normalize( -pos );
        vec3 N = normalize(Normal_Matrix * vNormal);
        //N must use the one pointing toward the viewer
        if(dot(N,E) < 0) N = -N;
        
        //global
        vec4 g_color = gAmbientProduct;
        
        //directional
        vec3 Ld = normalize( -Lightfrom );
        vec3 Hd = normalize( Ld + E );
        float d_attenuation = 1.0;
        // Compute terms in the illumination equation
        ambient = dAmbientProduct;
        d = max( dot(Ld, N), 0.0 );
        diffuse = d * dDiffuseProduct;
        s = pow( max(dot(N, Hd), 0.0), Shininess );
        specular = s * dSpecularProduct;
        if( dot(Ld, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        vec4 d_color = d_attenuation * (ambient + diffuse + specular);
        vec4 s_color = vec4(0.0, 0.0, 0.0, 0.0);
        //spotlight
        if(lightoption == 1){
            float s1_attenuation = 1.0;
            vec3 L1 = normalize( LightPosition.xyz - pos );
            vec3 Lf = normalize( towardpoint.xyz - LightPosition.xyz);
            vec3 H1 = normalize( L1 + E );
            if(dot(-L1, Lf) < cos(angle)){
                s1_attenuation = 0.0;
            }
            else{
                float d1 = sqrt(dot((LightPosition.xyz - pos), (LightPosition.xyz - pos)));
                s1_attenuation = pow(dot(-L1, Lf), exponent) * 1/(ConstAtt + LinearAtt*d1 + QuadAtt*d1*d1);
            }
            ambient = sAmbientProduct;
            d = max( dot(L1, N), 0.0 );
            diffuse = d * sDiffuseProduct;
            s = pow( max(dot(N, H1), 0.0), Shininess );
            specular = s * sSpecularProduct;
            if( dot(L1, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }
            s_color = s1_attenuation * (ambient + diffuse + specular);
        }
        //point source
        if(lightoption == 2){
            vec3 L2 = normalize( LightPosition.xyz - pos );
            vec3 H2 = normalize( L2 + E );
            float d2 = sqrt(dot((LightPosition.xyz - pos), (LightPosition.xyz - pos)));
            float s2_attenuation = 1/(ConstAtt + LinearAtt*d2 + QuadAtt*d2*d2);
            ambient = sAmbientProduct;
            d = max( dot(L2, N), 0.0 );
            diffuse = d * sDiffuseProduct;
            s = pow( max(dot(N, H2), 0.0), Shininess );
            specular = s * sSpecularProduct;
            if( dot(L2, N) < 0.0 ) {
                specular = vec4(0.0, 0.0, 0.0, 1.0);
            }
            s_color = s2_attenuation * (ambient + diffuse + specular);
        }
        color = g_color+d_color+s_color;
    }
    
}
