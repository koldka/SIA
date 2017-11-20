#version 400 core

uniform int width;
uniform int height;
uniform sampler2D normals_tex;
uniform sampler2D colors_tex;
uniform mat4 inv_projection_matrix;

uniform vec4 light_pos;
uniform vec3 light_col;

out vec4 out_color;

vec3 VSPositionFromDepth(vec2 texcoord, float z)
{
    // Get x/w and y/w from the viewport position
    vec4 positionNDC = vec4(2 * vec3(texcoord, z) - 1, 1.f);
    // Transform by the inverse projection matrix
    vec4 positionVS = inv_projection_matrix * positionNDC;
    // Divide by w to get the view-space position
    return positionVS.xyz / positionVS.w;
}


vec3 shade(vec3 N, vec3 L, vec3 V,
           vec3 color, float Ka, float Kd, float Ks,
           vec3 lightCol, float shininess){

    vec3 final_color = color * Ka * lightCol;	//ambient

    float lambertTerm = dot(N,L);		//lambert

    if(lambertTerm > 0.0) {
        final_color += color * Kd * lambertTerm * lightCol; 	//diffuse

        vec3 R = reflect(-L,N);
        float specular = pow(max(dot(R,V), 0.0), shininess);
        final_color +=  Ks * lightCol * specular;	//specular
    }

    return final_color;
}

void main() {
    vec2 texcoord;
    texcoord.x = gl_FragCoord.x / width;
    texcoord.y = gl_FragCoord.y / height;
    vec4 normal = texture(normals_tex, texcoord);
    //normal = normalize((normal - 1) * 2);
    vec4 color = texture(colors_tex, texcoord);
    vec3 pos_obj_cam = VSPositionFromDepth(texcoord, normal.a);

    vec3 l = vec3(light_pos.xyz - pos_obj_cam);
    out_color.rgb = shade(normalize(normal.xyz), normalize(l), -normalize(pos_obj_cam.xyz),
                          color.xyz, 0.2, 0.7, color.a, light_col/max(1,dot(l,l)*0.05), 5);

    out_color.a = 1.0;
}
