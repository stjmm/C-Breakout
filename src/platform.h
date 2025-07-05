#pragma once

#include <GLFW/glfw3.h>

typedef struct {
    GLFWwindow *handle;
    int width, height;
    int fb_width, fb_height;
    const char *title;
} Window;

typedef struct {
    int keys[GLFW_KEY_LAST + 1];
    int buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    int mouse_x, mouse_y;
    int mouse_dx, mouse_dy;
} Input;

typedef struct {
    float now;
    float delta;
} Time;

typedef struct {
    Window window;
    Input input;
    Time time;
} Platform;

void platform_init(Platform *platform, int width, int height, const char *title);
void platform_poll_events(Platform *platform);
void platform_swap_buffers(Platform *platform);
void platform_destroy(Platform *platform);
