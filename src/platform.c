#include <glad/glad.h>
#include "platform.h"

#include <stdio.h>

#include "util.h"

static void error_callback(int code, const char *desc) {
    fprintf(stderr, "GLFW error (%d): %s\n", code, desc);
}

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    Platform *platform = glfwGetWindowUserPointer(window);
    platform->window.fb_width = width;
    platform->window.fb_height = height;
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Platform *platform = glfwGetWindowUserPointer(window);
    if (key >= 0 && key <= GLFW_KEY_LAST) {
        platform->input.keys[key] = (action != GLFW_RELEASE);
    }
}
void platform_init(Platform *platform, int width, int height, const char *title) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        ERROR_EXIT("Failed to initialize GLFW.\n");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    platform->window.handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!platform->window.handle) {
        ERROR_EXIT("Failed to create GLFW window.\n");
    }

    platform->window.width = width;
    platform->window.height = height;
    platform->window.title = title;

    glfwSetWindowUserPointer(platform->window.handle, platform);
    glfwMakeContextCurrent(platform->window.handle);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        ERROR_EXIT("Failed to initialize GLAD.\n");
    }

    glfwGetFramebufferSize(platform->window.handle, &platform->window.fb_width, &platform->window.fb_height);
    glViewport(0, 0, platform->window.fb_width, platform->window.fb_height);

    printf("OpenGL: %s / %s", glGetString(GL_VENDOR), glGetString(GL_VERSION));

    glfwSetFramebufferSizeCallback(platform->window.handle, framebuffer_resize_callback);
    glfwSetKeyCallback(platform->window.handle, key_callback);

    glfwSetInputMode(platform->window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    platform->time.now = glfwGetTime();
}

void platform_poll_events(Platform *platform) {
    platform->input.mouse_dx = platform->input.mouse_dy = 0;

    glfwPollEvents();

    float t = glfwGetTime();
    platform->time.delta = t - platform->time.now;
    platform->time.now = t;
}

void platform_swap_buffers(Platform *platform) {
    glfwSwapBuffers(platform->window.handle);
}

void platform_destroy(Platform *platform) {
    glfwDestroyWindow(platform->window.handle);
    glfwTerminate();
}
