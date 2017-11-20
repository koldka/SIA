#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "warp.h"
#include "areaLight.h"
class AO : public Integrator
{
public:
    AO(const PropertyList &props) {
        m_sampleCount = props.getInteger("samples", 10);
        m_cosineWeighted = props.getBoolean("cosine", true);
        m_maxRecursion = props.getInteger("maxRecursion",4);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        //throw RTException("AO::Li() is not yet implemented!");
        Color3f radiance = Color3f::Zero();


        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (hit.foundIntersection())
        {
            Normal3f normal = hit.normal();
            Point3f pos = ray.at(hit.t());
            Vector3f u = normal.cross(Vector3f(0.0,0.0,1.0));
            Vector3f v = normal.cross(u);

            u.normalize();
            v.normalize();


            float randx = Eigen::internal::random<float>(0,1);
            float randy = Eigen::internal::random<float>(0,1);

            Point2f pos_hyper = Point2f(pos.x(), pos.y());
           // pos_hyper += u*randx;

            Vector3f d = Warp::squareToUniformHemisphere(pos_hyper);


            const Material* material = hit.shape()->material();

            const LightList &lights = scene->lightList();
            for(LightList::const_iterator it=lights.begin(); it!=lights.end(); ++it)
            {


                Vector3f lightDir;
                float dist;
                Point3f y;
                Point3f randPoint;



                 // lampe ponctuelle ou directionnelle
                 lightDir = (*it)->direction(pos, &dist);


                Vector3f nPos;
                Color3f attenuation = Color3f(1.f);
                for(int i = 0; i<m_sampleCount;i++){

                    Ray shadow_ray(pos + normal*Epsilon, lightDir, true);
                    Hit shadow_hit;
                    scene->intersect(shadow_ray,shadow_hit);

                    if(shadow_hit.t()<dist){
                        if(!shadow_hit.shape()->isEmissive())
                            attenuation = 0.5f * shadow_hit.shape()->material()->transmissivness();
                        if((attenuation <= 1e-6).all())
                            continue;
                        }
                }






    }}}






    std::string toString() const {
      return tfm::format("AO[\n"
                         "  samples = %f\n"
                         "  cosine-weighted = %s]\n",
                         m_sampleCount, 
                         m_cosineWeighted ? "true" : "false");
  }

private:
    int m_sampleCount;
    bool m_cosineWeighted;
     int m_maxRecursion;
};

REGISTER_CLASS(AO, "ao")
