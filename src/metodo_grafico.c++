#include "../SDL_Visor/SDL_Visor.hpp"
#include "parser.cpp"
#include <map>

std::map<String,vsr::Color*> Init_colors();

int main(){

    vsr::Screen window("Metodo gráfico de solución",600,500,SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    auto colores = Init_colors();    
    std::vector<PSR::Ecuation> ecuation_system;



    while (window.Handle_events()) {
        window.Clean_screen(*colores["blanco"]);
        window.Present_renderer();
    }

return 0;
}

std::map<String,vsr::Color*> Init_colors(){

    return std::map<String, vsr::Color*>({
        {"blanco",  new vsr::Color(255,255,255,255)},
        {"rojo",    new vsr::Color(255,0,0,0)},
        {"verde",   new vsr::Color(0,255,0,0)},
        {"azul",    new vsr::Color(0,0,255,0)},
        {"aqua",    new vsr::Color(0,100,100,0)},
        {"cyan",    new vsr::Color(0,255,255,0)}
        });

}





