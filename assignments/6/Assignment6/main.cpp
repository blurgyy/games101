#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    Scene scene(1280, 960);

    std::string splitMethodArg = "NAIVE";
    if(argc > 1){
        splitMethodArg = argv[1];
    }
    BVHAccel::SplitMethod splitMethod = BVHAccel::SplitMethod::NAIVE;
    if(splitMethodArg == "SAH"){
        splitMethod = BVHAccel::SplitMethod::SAH;
    }

    MeshTriangle bunny("../models/bunny/bunny.obj", 1, splitMethod);
    MeshTriangle armadillo("../models/armadillo/armadillo.obj", 1, splitMethod);


    // scene.Add(&bunny);
    scene.Add(&armadillo);
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 1));
    scene.Add(std::make_unique<Light>(Vector3f(20, 70, 20), 1));
    scene.buildBVH(1, splitMethod);

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " milliseconds\n";

    return 0;
}