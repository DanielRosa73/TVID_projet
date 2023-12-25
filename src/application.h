#pragma once

#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>

#include <stb_image.h>

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include "get_info.h"
#include "imageConverter.h"
#include "bob.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

namespace fs = std::filesystem;

namespace Application
{
    class Application
    {
        public:
            Application();
            ~Application();
            
            void Run();

            void Start();

            void exec(const char* cmd);

            void ShowStartupWindow();

            void Update();

            void LoadImage(const std::string& filename);

            void SaveImageAsPPM(const std::string& inputPath, const std::string& outputPath);

            inline GLFWwindow* GetWindow() { return window_; }

        private:
            GLFWwindow* window_;
            ImVec4 clear_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

            ImageInfo imageInfo_ = { 0, 0, 0, "" };

            GLuint textureID_;

            std::vector<std::string> imageVideoPathsPGM_;
            std::vector<std::string> imageVideoPathsPPM_;
            std::vector<std::string> imageVideoPathsBOB_;

            float ms_ = 0.0f;
            float fps_ = 0.0f;

            bool isVideoPGM_ = false;
            bool isVideoPPM_ = false;
            bool isVideoBOB_ = false;

            bool autoPlay_ = false;

            bool showExtensionWindow_ = true;
            const char* videoExtensions_[2] = {".m2v", ".ts" };
            int currentExtension_ = 0;

            std::vector<bool> progressiveFrame_;
            std::vector<bool> tffFrame_;
            std::vector<bool> rffFrame_;
    };
}