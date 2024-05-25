#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/vec4.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <cglm/mat3x2.h>

#include <stdio.h>
#include <stdlib.h>

typedef GLFWwindow mjoelnir_window;

struct mjoelnir_app {
    mjoelnir_window* window;
} mjoelnir_app;

mjoelnir_window* mjoelnir_CreateWindow(int width, int height, const char* title) {
    printf("--- Creating window ---\n");
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return glfwCreateWindow(800, 600, title, NULL, NULL);
}

struct mjoelnir_app* mjoelnir_Startup(int width, int height, const char* title) {
    printf("--- Initializing mjoelnir ---\n");
    struct mjoelnir_app* app = malloc(sizeof(mjoelnir_app));

    app->window = mjoelnir_CreateWindow(width, height, title);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    printf("Extension count: %d\n", extensionCount);

    return app;
}

bool mjoelnir_Run(struct mjoelnir_app* app) {
    bool running = !glfwWindowShouldClose(app->window);

    glfwPollEvents();

    return running;
}

void mjoelnir_Shutdown(struct mjoelnir_app* app) {
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

// void mjoelnir_Test()
// {
//     // Test some matrix calculations?!
//     vec3 v = { 2, 1, 0 };
//     mat3x2 m = {{1, 0}, {-1, -3}, {2, 1}};
//     vec2 r;
//     glm_mat3x2_mulv(m, v, r);
//     printf("%f, %f\n", r[0], r[1]);
// }