#include "application.h"

namespace Application
{
    Application::Application()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            fprintf(stderr, "Failed to initialize GLFW\n");

// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

        // Create window with graphics context
        window_ = glfwCreateWindow(1920, 1080, "TVID Project", nullptr, nullptr);
        if (window_ == nullptr)
            fprintf(stderr, "Failed to create GLFW window\n");
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != nullptr);
    }

    Application::~Application()
    {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    void Application::Run()
    {
        Start();

        while (!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            Update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color_.x * clear_color_.w, clear_color_.y * clear_color_.w, clear_color_.z * clear_color_.w, clear_color_.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window_);
        }
    }

    void Application::Start()
    {}

    std::string ReplaceFileExtension(const std::string& originalPath, const std::string& newExtension)
    {
        size_t lastDotIndex = originalPath.find_last_of(".");
        if (lastDotIndex == std::string::npos) {
            return originalPath + newExtension;
        }

        return originalPath.substr(0, lastDotIndex) + newExtension;
    }

    void Application::Update()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Menu"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".pgm", ".");
                }

                if (ImGui::MenuItem("Save"))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".ppm", ".");
                }

                if (ImGui::MenuItem("Exit"))
                {
                    glfwSetWindowShouldClose(window_, true);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                input_filePathName_ = filePathName;

                // Load the image
                LoadImage(input_filePathName_);
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                output_filePathName_ = const_cast<char*>(filePathName.c_str());

                // Save the image
                SaveImageAsPPM(output_filePathName_);
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if (textureID_ != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 20));
            ImGui::SetNextWindowSize(ImVec2(720, 720));
            if (ImGui::Begin("Image", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(textureID_)), ImGui::GetContentRegionAvail());
                ImGui::End();
            }
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(0, 20));
            ImGui::SetNextWindowSize(ImVec2(720, 720));
            if (ImGui::Begin("Image", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("No image loaded");
                ImGui::End();
            }
        }

        if (textureID_ != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(720, 20));
            ImGui::SetNextWindowSize(ImVec2(150, 720));
            if (ImGui::Begin("Image Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("Image Info");
                ImGui::Text("Width: %d", imageInfo_.width);
                ImGui::Text("Height: %d", imageInfo_.height);
                ImGui::Text("Depth: %d", imageInfo_.depth);
                ImGui::Text("Sampling Mode: %s", imageInfo_.samplingMode.c_str());
                ImGui::End();
            }
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(720, 20));
            ImGui::SetNextWindowSize(ImVec2(150, 720));
            if (ImGui::Begin("Image Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("No image loaded");
                ImGui::End();
            }
        }

        if (textureID_ != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(870, 20));
            ImGui::SetNextWindowSize(ImVec2(150, 720));
            if (ImGui::Begin("Image To PPM", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                if (ImGui::Button("Convert"))
                {
                    std::string outputPath = ReplaceFileExtension(input_filePathName_, ".ppm");

                    ConvertPGMtoPPM(input_filePathName_, outputPath);

                    std::cout << "Saved PPM image to " << outputPath << std::endl;

                    LoadImage(outputPath);
                }

                ImGui::End();
            }
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(720, 20));
            ImGui::SetNextWindowSize(ImVec2(300, 720));
            if (ImGui::Begin("Image To PPM", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("No image loaded");
                ImGui::End();
            }
        }
    }

    void Application::LoadImage(const std::string& filename)
    {
        int imageWidth, imageHeight, imageChannels;
        unsigned char* imageData = stbi_load(filename.c_str(), &imageWidth, &imageHeight, &imageChannels, 0);

        if (imageData == nullptr)
        {
            fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
            return;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (imageChannels == 1)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, imageWidth, imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, imageData);
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }
        else if (imageChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        }
        else if (imageChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        }

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL error: " << err << std::endl;
        }

        textureID_ = textureID;
        imageInfo_.width = imageWidth;
        imageInfo_.height = imageHeight;
        imageInfo_.depth = imageChannels;

        stbi_image_free(imageData);
    }

    void Application::SaveImageAsPPM(const std::string& outputPath)
    {
        std::vector<unsigned char> ppmData(imageInfo_.width * imageInfo_.height * 3);

        std::ifstream input(input_filePathName_, std::ios::binary);
        if (!input.is_open())
        {
            std::cerr << "Could not open the input file." << std::endl;
            return;
        }

        std::string line;
        std::getline(input, line);
        if (line != "P5")
        {
            std::cerr << "Unsupported image format!" << std::endl;
            return;
        }

        std::getline(input, line);
        while (line[0] == '#')
        {
            std::getline(input, line);
        }

        std::istringstream iss(line);
        int width, height, maxval;
        iss >> width >> height;
        input >> maxval;
        input.get();

        std::vector<unsigned char> image(width * height);
        std::vector<unsigned char> colorImage(width * height * 3);

        input.read(reinterpret_cast<char*>(image.data()), image.size());

        for (int i = 0; i < image.size(); i++)
        {
            ppmData[i * 3] = image[i];
            ppmData[i * 3 + 1] = image[i];
            ppmData[i * 3 + 2] = image[i];
        }

        std::ofstream outFile(outputPath, std::ios::binary);
        if (outFile.is_open())
        {
            outFile << "P6\n" << imageInfo_.width << " " << imageInfo_.height << "\n255\n";
        
            outFile.write(reinterpret_cast<char*>(ppmData.data()), ppmData.size());

            outFile.close();
            std::cout << "Saved PPM image to " << outputPath << std::endl;
        }
        else
        {
            std::cerr << "Could not open the output file path for writing: " << outputPath << std::endl;
        }
    }
}