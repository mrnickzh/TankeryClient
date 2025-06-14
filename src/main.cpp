#include <emscripten.h>
#include <emscripten/html5.h>
#include <thread>
#include <raylib.h>

int windowWidth = 800;
int windowHeight = 600;

EM_BOOL onResize(int, const EmscriptenUiEvent* e, void*) {
    windowWidth = e->windowInnerWidth;
    windowHeight = e->windowInnerHeight;
    SetWindowSize(windowWidth, windowHeight);
    return EM_TRUE;
}

void mainLoop() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
    EndDrawing();
}

int main() {
    bool ticklooprunning = true;

    emscripten_get_canvas_element_size("#canvas", &windowWidth, &windowHeight);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onResize);

    InitWindow(windowWidth, windowHeight, "raylib [core] example - basic window");

    std::thread tick([&]() {
        int tps = 40;
        int mps = 1000 / tps;
        long long ltt = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        while (ticklooprunning) {
            long long now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            long long elapsed = now - ltt;

            if (elapsed >= mps) {
                int ticksToRun = (int) (elapsed / mps);
                ltt += ticksToRun * mps;

                for (int i = 0; i < ticksToRun; i++) {
                    //tickevent
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
         }
    });
    emscripten_set_main_loop(mainLoop, 0, 1);
    ticklooprunning = false;
    tick.join();
    CloseWindow();
    return 0;
}