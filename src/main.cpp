#include <SDL.h>                 // veya: #include <SDL_main.h>
#include "engine/Application.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;      // kullan�lm�yor uyar�s�n� susturmak i�in
    Erlik::Application app;
    return app.run();
}
