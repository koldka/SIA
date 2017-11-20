#include "material.h"
#include "warp.h"
#include <Eigen/Geometry>
#include <iostream>
#include <math.h>
#include<limits.h>

void Material::loadTextureFromFile(const std::string& fileName)
{
    if (fileName.size()==0)
        std::cerr << "Material error : no texture file name provided" << std::endl;
    else
        m_texture = new Bitmap(fileName);
}

Diffuse::Diffuse(const PropertyList &propList)
{
    m_diffuseColor = propList.getColor("diffuse",Color3f(0.2));
    m_alphaX = propList.getFloat("alphaX",0.2);
    m_alphaY = propList.getFloat("alphaY",0.1);

    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTextureFromFile(filepath.str());
        setTextureScale(propList.getFloat("scale",1));
        setTextureMode(TextureMode(propList.getInteger("mode",0)));
    }
}

Color3f Diffuse::diffuseColor(const Vector2f& uv) const
{
    if(texture() == nullptr)
        return m_diffuseColor;

    float u = uv[0];
    float v = uv[1];

    // Take texture scaling into account
    u /= textureScaleU();
    v /= textureScaleV();

    // Compute pixel coordinates
    const int i = int(fabs(u - floor(u)) * texture()->cols());
    const int j = int(fabs(v - floor(v)) * texture()->rows());

    Color3f fColor = (*texture())(j,i);

    // Compute color
    switch(textureMode())
    {
    case MODULATE:
        return  fColor * m_diffuseColor;
    case REPLACE:
        return fColor;
    }
    return fColor;
}



Vector3f Ward::is(const Normal3f normal,  Vector3f i ) const
{
    Vector3f vec;
    Vector3f o;
    float pdf;
    float u = Eigen::internal::random<float>(0,1);
    float v = Eigen::internal::random<float>(0,1);
    float phiH = atan((m_alphaY/m_alphaX)*tan(2.0*M_PI*v));
    float randRay = Eigen::internal::random<float>(0,m_specularColor.mean()+m_diffuseColor.mean());


    Vector3f d = Vector3f(0.f,1.f,0.f);
    Vector3f x = d-(d.dot(normal))*normal;
    x.normalize();
    Vector3f y = normal.cross(x);

    if(randRay > m_diffuseColor.mean()){

        float thetaH =  atan(sqrt(-log(u)/((cos(phiH)*cos(phiH))/(m_alphaX*m_alphaX) + (sin(phiH)*sin(phiH))/(m_alphaY*m_alphaY))));

        if((v>0.25) && (v<=0.5)){
            phiH +=M_PI;
        }else if((v>0.5)&&(v<0.75)){
            phiH -=M_PI;
        }



        Vector3f h = sin(thetaH)*cos(phiH)*x + sin(thetaH)*sin(phiH) * y + cos(thetaH)*normal;
        o = 2.0*(i.dot(h))*h-i;


    }
    else{

        vec = Warp::squareToCosineHemisphere(Point2f(Eigen::internal::random<float>(0,1),Eigen::internal::random<float>(0,1)));
        //sa densité
        pdf = Warp::squareToCosineHemispherePdf(vec);

        o =  vec.x() * x + vec.y() * y + vec.z() * normal;
    }
    o.normalize();
    return o;
}

REGISTER_CLASS(Diffuse, "diffuse")
