#include "SDL_Visor/SDL_Visor.hpp"

int main(int argc, char const *argv[])
{
    vsr::Screen window("Metodo grafico",600,500,SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    

    while (window.Handle_events())
    {
        
    }
    



    return 0;
}

