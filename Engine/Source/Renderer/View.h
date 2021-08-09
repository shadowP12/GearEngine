#pragma once

namespace gear {
    class Renderer;
    class Scene;
    class View {
    public:
        View(Renderer* renderer);

        ~View();

    private:
        Renderer* _renderer = nullptr;
    };
}