#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    if(splitMethod == SplitMethod::NAIVE){
        root = recursiveBuild(primitives);
    }
    else if(splitMethod == SplitMethod::SAH){
        root = SAHPartition(primitives);
    }

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete(%s): \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        splitMethod == SplitMethod::NAIVE ? "NAIVE" : "SAH", 
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

BVHBuildNode *BVHAccel::SAHPartition(std::vector<Object *> objects){
    // printf("%lu\n", objects.size());
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1)
    {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = SAHPartition(std::vector{objects[0]});
        node->right = SAHPartition(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else
    {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        const int nBuckets = 16;
        int cnt[nBuckets];
        Bounds3 aabb[nBuckets];
        std::vector<Object *> bucket[nBuckets];
        float minCost = std::numeric_limits<float>::max();
        int split = -1;

        int dim = centroidBounds.maxExtent();
        for(int i = 0; i < nBuckets; ++ i){
            cnt[i] = 0;
            aabb[i] = Bounds3();
            bucket[i].clear();
        }
        float dimExtent = bounds.Diagonal()[dim];
        for(auto obj : objects){
            float position = obj->getBounds().Centroid()[dim] - bounds.pMin[dim];
            int bucIndex = int(position * nBuckets / dimExtent);
            aabb[bucIndex] = Union(aabb[bucIndex], obj->getBounds());
            bucket[bucIndex].push_back(obj);
            cnt[bucIndex] ++;
        }
        for(int i = 1; i < nBuckets; ++ i){
            cnt[i] += cnt[i-1];
            int lCnt = cnt[i-1];
            int rCnt = objects.size() - lCnt;
            Bounds3 lBound, rBound;
            for(int j = 0; j < i; ++ j)
                lBound = Union(lBound, aabb[j]);
            for(int j = i; j < nBuckets; ++ j)
                rBound = Union(rBound, aabb[j]);
            float ithCost = lBound.SurfaceArea() * lCnt + rBound.SurfaceArea() * rCnt;
            if(ithCost < minCost){
                minCost = ithCost;
                split = i;
            }
        }

        std::vector<Object *> leftShapes, rightShapes;
        for(int i = 0; i < nBuckets; ++ i){
            if(i < split){
                leftShapes.insert(leftShapes.end(), bucket[i].begin(), bucket[i].end());
            }
            else {
                rightShapes.insert(rightShapes.end(), bucket[i].begin(), bucket[i].end());
            }
        }
        assert(objects.size() == (leftShapes.size() + rightShapes.size()));

        node->left = SAHPartition(leftShapes);
        node->right = SAHPartition(rightShapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

    Intersection BVHAccel::Intersect(const Ray &ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection ret;
    Vector3f invDir = Vector3f(1/ray.direction.x, 1/ray.direction.y, 1/ray.direction.z);
    std::array<int, 3> dirIsNeg{int(ray.direction.x < 0), int(ray.direction.y < 0), int(ray.direction.z < 0)};
    if(!node->bounds.IntersectP(ray, invDir, dirIsNeg)){
        return ret;
    }
    // if is leaf node
    if(node->left == nullptr && node->right == nullptr){
        return node->object->getIntersection(ray);
    }
    // is intermidiate node
    Intersection lRet = getIntersection(node->left, ray);
    Intersection rRet = getIntersection(node->right, ray);
    if(lRet.happened){
        ret = lRet;
    }
    if(rRet.happened){
        if(rRet.distance < ret.distance){
            ret = rRet;
        }
    }
    return ret;
}