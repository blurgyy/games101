#include <iostream>
#include <exception>
#include <cstdlib>

#include <Application.hpp>
#include <global.hpp>

int main(){
    Application app;
    try{
        app.run();
    } catch(const std::exception& e){
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
