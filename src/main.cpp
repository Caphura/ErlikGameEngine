#include <SDL.h>                 // veya: #include <SDL_main.h>
#include "engine/Application.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;      // kullanýlmýyor uyarýsýný susturmak için
    Erlik::Application app;
    return app.run();
}
