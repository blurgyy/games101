//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TODO Implement Path Tracing Algorithm here
    if(depth > this->maxDepth){
        return Vector3f(0,0,0);
    }
    Intersection intersection = this->intersect(ray);
    Material *m = intersection.m;
    Vector3f hitColor = this->backgroundColor;

    if (intersection.happened){
        if(m->hasEmission()){
            return m->getEmission();
        }
        Vector3f L_dir = 0, L_indir = 0;

        Vector3f p = intersection.coords;
        Vector3f wo = -ray.direction;
        Vector3f N = intersection.normal; // normal
        
        Intersection inter;
        float pdf_light;
        sampleLight(inter, pdf_light);
        Vector3f x = inter.coords;
        Vector3f ws = normalize(x - p);
        Vector3f NN = inter.normal;
        Vector3f emit = inter.emit;
        Ray checker_ray(p, ws);
        if((this->intersect(checker_ray).coords - x).norm() < EPSILON){
            Vector3f f_r = m->eval(wo, ws, N);
            L_dir = emit * f_r * dotProduct(ws, N) * dotProduct(-ws, NN)
                / dotProduct(x-p, x-p) / pdf_light;
            // printf("(%f)\n", dotProduct(ws, N));
            // printf("%f\n", pdf_light);
            // printf("%f\n", L_dir.norm());
            // std::cout << m->eval(wo, ws, N) << std::endl;
        }
        if(get_random_float() < RussianRoulette){
            Vector3f wi = m->sample(wo, N);
            checker_ray = Ray(p, wi);
            Intersection diffuse_intersection = this->intersect(checker_ray);
            if(diffuse_intersection.happened && !diffuse_intersection.m->hasEmission()){
                Vector3f f_r = m->eval(wo, wi, N);
                float pdf = m->pdf(wo, wi, N);
                L_indir = castRay(checker_ray, depth) * f_r * dotProduct(wi, N)
                    / pdf / RussianRoulette;
            }
        }
        
        hitColor = L_dir + L_indir;
    }

    return hitColor;
}
