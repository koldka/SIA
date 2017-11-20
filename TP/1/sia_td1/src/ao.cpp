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
        Color3f intensity = Color3f::Zero();


        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (hit.foundIntersection())
        {
            Normal3f normal = hit.normal();
            Point3f pos = ray.at(hit.t());
             Vector3f u;
            for(int i=0; i<m_sampleCount; i++){
                if(normal.y() == 0 && normal.z() == 0)
                   u = normal.cross(Vector3f(0.0,0.0,1.0));
                else
                   u = normal.cross(Vector3f(1.0,0.0,0.0));

                u.normalize();
                Vector3f v = normal.cross(u);
                v.normalize();


                float randx = Eigen::internal::random<float>(0,1);
                float randy = Eigen::internal::random<float>(0,1);

                Point2f pos_hyper = Point2f(randx, randy);

                Vector3f d;
                if (m_cosineWeighted) {
                    d = Warp::squareToCosineHemisphere(pos_hyper);
                }
                else {
                    d = Warp::squareToUniformHemisphere(pos_hyper);
                }

                Vector3f p = d.x() * u + d.y() * v + d.z() * normal;
                Ray shadow_ray(pos + normal*Epsilon, p, true);
                Hit shadow_hit;
                scene->intersect(shadow_ray,shadow_hit);

                double pdf;
                if (m_cosineWeighted) {
                    pdf= Warp::squareToCosineHemispherePdf(d);
                }
                else {
                    pdf= Warp::squareToUniformHemispherePdf(d);
                }

                if(!shadow_hit.foundIntersection())
                    intensity += Color3f(1.0f)*((p.dot(normal) / (p.norm() * normal.norm())) / M_PI) / pdf;
            }
            intensity = intensity/m_sampleCount;

        }
        return intensity;
    }







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
