#include "material.h"
#include <Eigen/Dense>

Ward::Ward(const PropertyList &propList)
    : Diffuse(propList.getColor("diffuse",Color3f(0.2)))
{
    m_reflectivity = propList.getColor("reflectivity",Color3f(0.0));
    m_transmissivness = propList.getColor("transmissivness",Color3f(0.0));
    m_etaA = propList.getFloat("etaA",1);
    m_etaB = propList.getFloat("etaB",1);
    m_specularColor = propList.getColor("specular",Color3f(0.9));//ro
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

Color3f Ward::brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    //throw RTException("Ward::brdf() is not yet implemented!");
    Vector3f y;
    Vector3f x;
    Vector3f d = Vector3f(0.0,1.0,0.0);
    x=d-(d.dot(normal))*normal;
    x.normalize();
    y = normal.cross(x);
    Vector3f h = (lightDir+viewDir)/((lightDir+viewDir).norm());

    Color3f res = diffuseColor(uv) / M_PI;
    double tmp = lightDir.dot(normal)*viewDir.dot(normal);
    if (tmp<0) {
       return 0;
    }
    Color3f fr  = m_specularColor/(4*M_PI*m_alphaX*m_alphaY*sqrt(tmp));
    fr *= exp(-((h.dot(x)/m_alphaX)*(h.dot(x)/m_alphaX) + (h.dot(y)/m_alphaY)*(h.dot(y)/m_alphaY)) / (h.dot(normal) * h.dot(normal)));
    res += fr;
    return res;

}

Color3f Ward::premultBrdf(const Vector3f& i, const Vector3f& o, const Normal3f& normal, const Vector2f& uv) const
{

    Vector3f h = (i+o)/(i+o).norm();

    if(i.dot(normal)*o.dot(normal)>0){
        Color3f res = m_specularColor*(h.dot(i)*h.dot(normal)*h.dot(normal)*h.dot(normal) * sqrt(o.dot(normal)/(i.dot(normal))));
        return res;
}else
        return Color3f(0.f);

}

std::string Ward::toString() const {
    return tfm::format(
        "Ward [\n" 
        "  diffuse color = %s\n"
        "  specular color = %s\n"
        "  alphaX = %f  alphaY = %f\n"
        "]", m_diffuseColor.toString(),
             m_specularColor.toString(),
             m_alphaX, m_alphaY);
}

REGISTER_CLASS(Ward, "ward")
