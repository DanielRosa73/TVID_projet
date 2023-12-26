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
        window_ = glfwCreateWindow(1130, 740, "TVID Project", nullptr, nullptr);
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
        io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
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

            if (showExtensionWindow_) // Check if we need to show the startup window
            {
                ShowStartupWindow(); // Call the function that shows the startup window
            }

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

    std::string changeExtension(const std::string& originalPath, const std::string& newExtension)
    {
        std::filesystem::path pathObj(originalPath);

        // Change the extension
        pathObj.replace_extension(newExtension);

        std::string newPath = pathObj.string();
        size_t pos = newPath.find("/pgm/");
        if (pos != std::string::npos)
        {
            newPath.replace(pos, 5, "/ppm/");
        }

        return newPath;
    }

    void Application::exec(const char* cmd)
    {
        std::array<char, 128> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            imageVideoPathsPGM_.push_back(buffer.data());
        }
    }

    std::vector<std::string> findFilesWithExtension(const std::string& path, const std::string& extension)
    {
        std::vector<std::string> pgmFiles;
        
        try {
            if (fs::exists(path) && fs::is_directory(path)) {
                for (const auto& entry : fs::directory_iterator(path)) {
                    if (entry.is_regular_file() && entry.path().extension() == extension) {
                        pgmFiles.push_back(entry.path().string());
                    }
                }
            } else {
                std::cerr << "Path does not exist or is not a directory: " << path << '\n';
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << e.what() << '\n';
        }

        return pgmFiles;
    }

    std::string extractFileNameWithoutExtension(const std::string& filePath)
    {
        size_t lastSlash = filePath.find_last_of("/\\");
        std::string fileNameWithExt = filePath.substr(lastSlash + 1);

        size_t lastDot = fileNameWithExt.find_last_of(".");
        std::string fileNameWithoutExt = fileNameWithExt.substr(0, lastDot);

        return fileNameWithoutExt;
    }

    std::vector<std::string> orderFilesByNumber(const std::vector<std::string>& files)
    {
        std::vector<std::string> orderedFiles;

        std::map<int, std::string> orderedMap;

        for (const auto& file : files)
        {
            std::string fileName = extractFileNameWithoutExtension(file);

            std::string numberString = fileName.substr(fileName.find_last_of("_") + 1);

            int number = std::stoi(numberString);

            orderedMap[number] = file;
        }

        for (const auto& [key, value] : orderedMap)
        {
            orderedFiles.push_back(value);
        }

        return orderedFiles;
    }

    std::string findFolderName(const std::string& path)
    {
        std::size_t lastSlashPos = path.find_last_of("/");

        if (lastSlashPos == std::string::npos || lastSlashPos == 0) {
            return "";
        }

        std::size_t secondLastSlashPos = path.find_last_of("/", lastSlashPos - 1);

        if (secondLastSlashPos == std::string::npos) {
            return path.substr(0, lastSlashPos);
        }

        return path.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);
    }

    int countFilesInFolder(const std::string& path)
    {
        int fileCount = 0;

        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                fileCount++;
            }
        }

        return fileCount;
    }

    static char pid_[128] = "";

    void Application::ShowStartupWindow()
    {
        // This function is now correctly placed inside the ImGui frame scope
        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        ImGui::Begin("Welcome", &showExtensionWindow_, ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Welcome to the Video Loader App!");
        ImGui::Text("Please select the video file extension you wish to load:");

        if (ImGui::Combo("Video File Extension", &currentExtension_, videoExtensions_, IM_ARRAYSIZE(videoExtensions_)))
        {
            std::string selectedExtension = videoExtensions_[currentExtension_];

            std::cout << "Selected extension: " << selectedExtension << std::endl;
        }

        if (ImGui::Button("OK"))
        {
            showExtensionWindow_ = false;
        }

        ImGui::End();
    }

    void Application::Update()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Menu"))
            {
                if (ImGui::MenuItem("Open Video"))
                {
                    ImGuiFileDialog::Instance()->OpenDialog("ChooseVideoDlgKey", "Choose Video", videoExtensions_[currentExtension_], ".");
                }

                if (ImGui::MenuItem("Change Video Extension"))
                {
                    showExtensionWindow_ = true;
                }

                if (ImGui::MenuItem("Exit"))
                {
                    glfwSetWindowShouldClose(window_, true);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (textureID_ != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(720, 20));
            ImGui::SetNextWindowSize(ImVec2(200, 720));
            if (ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("Image Info");
                ImGui::Text("Width: %d", imageInfo_.width);
                ImGui::Text("Height: %d", imageInfo_.height);
                ImGui::Text("Depth: %d", imageInfo_.depth);
                ImGui::Text("Sampling Mode: %s", imageInfo_.samplingMode.c_str());

                ImGui::Separator();

                ImGui::Text("Fps and Ms Info of Video");
                ImGui::Text("ms: %f", ms_);
                ImGui::SliderFloat("ms", &ms_, 0.1f, 1000.0f);
                ImGui::Text("fps: %f", 1000.0f / ms_);

                ImGui::Separator();

                ImGui::InputText("Enter Pid", pid_, IM_ARRAYSIZE(pid_));

                ImGui::Separator();

                ImGui::Checkbox("Auto Play", &autoPlay_);

                ImGui::End();
            }
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(720, 20));
            ImGui::SetNextWindowSize(ImVec2(200, 720));
            if (ImGui::Begin("Image Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("No image loaded");
                ImGui::End();
            }
        }

        if (textureID_ != 0)
        {
            ImGui::SetNextWindowPos(ImVec2(920, 20));
            ImGui::SetNextWindowSize(ImVec2(210, 720));
            if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                if (ImGui::Button("Convert Video to PPM"))
                {
                    if (imageVideoPathsPGM_.size() == 0)
                    {
                        std::cerr << "No PGM files found." << std::endl;
                        std::cerr << "Please convert the video to PGM first (Load a video)." << std::endl;
                        ImGui::End();
                        return;
                    }

                    imageVideoPathsPPM_.clear();

                    isVideoPGM_ = false;
                    isVideoPPM_ = true;
                    isVideoBOB_ = false;
                    isVideoALT_ = false;

                    std::string command = "mkdir -p ../../test_images/ppm/" + findFolderName(imageVideoPathsPGM_[0]) + "/";
                    exec(command.c_str());

                    if (countFilesInFolder("../../test_images/ppm/" + findFolderName(imageVideoPathsPGM_[0]) + "/") == imageVideoPathsPGM_.size())
                    {
                        std::cout << "PPM files already exist." << std::endl;

                        for (const std::string& pgmFilePath : imageVideoPathsPGM_)
                        {
                            std::string ppmFilePath = changeExtension(pgmFilePath, ".ppm");
                            imageVideoPathsPPM_.push_back(ppmFilePath);
                        }
                    }
                    else
                    {
                        for (const std::string& pgmFilePath : imageVideoPathsPGM_)
                        {
                            try
                            {
                                std::string ppmFilePath = changeExtension(pgmFilePath, ".ppm");
                                ConvertPGMtoPPM(pgmFilePath, ppmFilePath);
                                std::cout << "Converted " << pgmFilePath << " to " << ppmFilePath << std::endl;
                                imageVideoPathsPPM_.push_back(ppmFilePath);
                            }
                            catch (const std::exception& e)
                            {
                                std::cerr << "Error converting to PPM: " << e.what() << '\n';
                            }
                        }
                    }

                    std::cout << "Conversion to PPM completed." << std::endl;
                }

                if (ImGui::Button("BOB Deinterlacing"))
                {
                    if (imageVideoPathsPPM_.size() == 0)
                    {
                        std::cerr << "No PPM files found." << std::endl;
                        std::cerr << "Please convert the video to PPM first." << std::endl;
                        ImGui::End();
                        return;
                    }

                    imageVideoPathsBOB_.clear();

                    int counter = 0;

                    isVideoPGM_ = false;
                    isVideoPPM_ = false;
                    isVideoBOB_ = true;
                    isVideoALT_ = false;

                    std::string folder_to_create = "../../test_images/bob/" + findFolderName(imageVideoPathsPPM_[0]) + "/";
                    std::string command = "mkdir -p " + folder_to_create;
                    exec(command.c_str());

                    int i = 0;
                    for (const std::string& ppmFilePath : imageVideoPathsPPM_)
                    {
                        try
                        {
                            // Handle progressive frame
                            if (progressiveFrame_[i])
                            {
                                imageVideoPathsBOB_.push_back(ppmFilePath);
                            }
                            else
                            {
                                std::string bobFilePath_1 = "../../test_images/bob/" + folder_to_create + std::to_string(counter) + ".ppm";
                                std::string bobFilePath_2 = "../../test_images/bob/" + folder_to_create + std::to_string(counter + 1) + ".ppm";

                                counter += 2;
                                BobOutput bobOutput = { bobFilePath_1, bobFilePath_2 };
                                
                                bob_deinterlacing(ppmFilePath, bobOutput, tffFrame_[i], rffFrame_[i]);
                                std::cout << "Converted to BOB: " << bobFilePath_1 << " and " << bobFilePath_2 << std::endl;
                                imageVideoPathsBOB_.push_back(bobFilePath_1);
                                imageVideoPathsBOB_.push_back(bobFilePath_2);
                            }

                            i++;
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Error converting to BOB: " << e.what() << '\n';
                        }
                    }

                    std::cout << "Conversion to BOB completed." << std::endl;
                }

                if (ImGui::Button("Alternative Deinterlacing"))
                {
                    if (imageVideoPathsPPM_.size() == 0)
                    {
                        std::cerr << "No PPM files found." << std::endl;
                        std::cerr << "Please convert the video to PPM first." << std::endl;
                        ImGui::End();
                        return;
                    }
                    imageVideoPathALT_.clear();

                    int counter = 0;

                    isVideoPGM_ = false;
                    isVideoPPM_ = false;
                    isVideoBOB_ = false;
                    isVideoALT_ = true;

                    std::string folder_to_create = "../../test_images/alt/" + findFolderName(imageVideoPathsPPM_[0]) + "/";
                    std::string command = "mkdir -p " + folder_to_create;
                    exec(command.c_str());

                    int i = 0;
                    for (const std::string& ppmFilePath : imageVideoPathsPPM_)
                    {
                        try
                        {
                            // Handle progressive frame
                            if (progressiveFrame_[i])
                            {
                                imageVideoPathALT_.push_back(ppmFilePath);
                            }
                            else
                            {
                                std::string altFilePath_1 = "../../test_images/alt/" + folder_to_create + std::to_string(counter) + ".ppm";
                                std::string altFilePath_2 = "../../test_images/alt/" + folder_to_create + std::to_string(counter + 1) + ".ppm";

                                counter += 2;
                                AltOutput AltOutput = {altFilePath_1, altFilePath_2};
                                
                                alt_deinterlacing(ppmFilePath, AltOutput, tffFrame_[i], rffFrame_[i]);
                                std::cout << "Converted to Improved Bob: " << altFilePath_1 << " and " << altFilePath_2<< std::endl;
                                imageVideoPathALT_.push_back(altFilePath_1);
                                imageVideoPathALT_.push_back(altFilePath_2);
                            }

                            i++;
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << "Error converting to BOB: " << e.what() << '\n';
                        }
                    }

                }
            }

            ImGui::End();
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(930, 20));
            ImGui::SetNextWindowSize(ImVec2(210, 720));
            if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
            {
                ImGui::Text("No image loaded");
                ImGui::End();
            }
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseVideoDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                std::cout << "Video file path: " << filePathName << std::endl;

                isVideoPGM_ = true;
                isVideoPPM_ = false;
                isVideoBOB_ = false;
                isVideoALT_ = false;

                // Load the video
                try
                {
                    exec("rm -rf flags.txt");
                    exec("rm -rf prog_seq.txt");

                    if (currentExtension_ == 1)
                    {
                        std::cout << "PID: " << pid_ << std::endl;
                        std::string str_pid = std::string(pid_);
                        std::string command = "../../tools/src/mpeg2dec -v -t " + str_pid + " -o pgm " + filePathName;
                        std::cout << command << std::endl;
                        exec(command.c_str());
                    }
                    else
                    {
                        std::string command = "../../tools/src/mpeg2dec -v -o pgm " + filePathName;
                        exec(command.c_str());
                    }

                    std::string video_name = extractFileNameWithoutExtension(filePathName);
                    std::string command = "mkdir -p ../../test_images/pgm/" + video_name + "/";
                    exec(command.c_str());

                    command = "mv *.pgm ../../test_images/pgm/" + video_name + "/";
                    exec(command.c_str());

                    command = "rm -rf *.pgm";
                    exec(command.c_str());

                    imageVideoPathsPGM_ = orderFilesByNumber(findFilesWithExtension("../../test_images/pgm/" + video_name + "/", ".pgm"));

                    std::cout << "Found " << imageVideoPathsPGM_.size() << " PGM files." << std::endl;

                    std::string path = "flags.txt";
                    std::ifstream file(path);

                    progressiveFrame_.clear();
                    tffFrame_.clear();
                    rffFrame_.clear();

                    progressiveFrame_.resize(imageVideoPathsPGM_.size());
                    tffFrame_.resize(imageVideoPathsPGM_.size());
                    rffFrame_.resize(imageVideoPathsPGM_.size());
                    
                    std::string fps_string;
                    std::string progressiveFrame_string;
                    std::string tffFrame_string;
                    std::string rffFrame_string;

                    for (int i = 0; i < imageVideoPathsPGM_.size(); i++)
                    {
                        std::getline(file, fps_string);

                        if (fps_string.empty() || file.fail())
                        {
                            std::cerr << "Could not read FPS from file: " << path << std::endl;
                            fps_ = 25.0f;
                        }
                        else
                        {
                            fps_ = std::stof(fps_string);
                        }

                        ms_ = 1000.0f / fps_;

                        std::getline(file, progressiveFrame_string);
                        std::getline(file, tffFrame_string);
                        std::getline(file, rffFrame_string);

                        progressiveFrame_[i] = progressiveFrame_string[0] == '1';
                        tffFrame_[i] = tffFrame_string[0] == '1';
                        rffFrame_[i] = rffFrame_string[0] == '1';
                    }

                    file.close();

                    path = "prog_seq.txt";

                    std::ifstream file2(path);

                    pseqFrame_.clear();

                    pseqFrame_.resize(imageVideoPathsPGM_.size());

                    std::string pseqFrame_string;

                    for (int i = 0; i < imageVideoPathsPGM_.size(); i++)
                    {
                        std::getline(file2, pseqFrame_string);

                        if (pseqFrame_string.empty() || file2.fail())
                        {
                            std::cerr << "Could not read PSEQ from file: " << path << std::endl;
                            pseqFrame_[i] = false;
                        }
                        else
                        {
                            pseqFrame_[i] = pseqFrame_string[0] == '1';
                        }
                    }
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }

            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::SetNextWindowPos(ImVec2(0, 20));
        ImGui::SetNextWindowSize(ImVec2(720, 720));
        if (ImGui::Begin("Video", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            if (imageVideoPathsPGM_.size() > 0)
            {
                static float lastTime = 0.0f;
                float currentTime = ImGui::GetTime();
                fps_ = ms_ / 1000.0f;

                static int currentImageIndex = -1;
                int previousImageIndex = currentImageIndex;

                if (autoPlay_)
                {
                    if (currentTime - lastTime >= fps_)
                    {
                        lastTime = currentTime;

                        currentImageIndex = (currentImageIndex + 1) % imageVideoPathsPGM_.size();

                        if (isVideoPGM_)
                        {
                            std::string imagePath = imageVideoPathsPGM_[currentImageIndex];

                            LoadImage(imagePath);
                        }

                        if (isVideoPPM_)
                        {
                            std::string imagePath = imageVideoPathsPPM_[currentImageIndex];

                            LoadImage(imagePath);
                        }

                        if (isVideoBOB_)
                        {
                            std::string imagePath = imageVideoPathsBOB_[currentImageIndex];

                            LoadImage(imagePath);
                        }
                        if (isVideoALT_)
                        {
                            std::string imagePath = imageVideoPathALT_[currentImageIndex];

                            LoadImage(imagePath);
                        }
                    }
                }
                else
                {

                    ImGui::SliderInt("Image Index", &currentImageIndex, 0, imageVideoPathsPGM_.size() - 1);

                    if (currentImageIndex != previousImageIndex)
                    {
                        if (isVideoPGM_)
                        {
                            std::string imagePath = imageVideoPathsPGM_[currentImageIndex];

                            LoadImage(imagePath);
                        }

                        if (isVideoPPM_)
                        {
                            std::string imagePath = imageVideoPathsPPM_[currentImageIndex];

                            LoadImage(imagePath);
                        }

                        if (isVideoBOB_)
                        {
                            std::string imagePath = imageVideoPathsBOB_[currentImageIndex];

                            LoadImage(imagePath);
                        }

                        if (isVideoALT_)
                        {
                            std::string imagePath = imageVideoPathALT_[currentImageIndex];

                            LoadImage(imagePath);
                        }
                    }
                }

                if (textureID_ != 0)
                {
                    ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(textureID_)), ImGui::GetContentRegionAvail());
                }
                else
                {
                    ImGui::Text("No image loaded");
                }
            }
            else
            {
                ImGui::Text("No video loaded");
            }

            ImGui::End();
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

        if (imageChannels == 1)
        {
            imageInfo_.samplingMode = "Grayscale";
        }
        else if (imageChannels == 3)
        {
            imageInfo_.samplingMode = "RGB";
        }
        else if (imageChannels == 4)
        {
            imageInfo_.samplingMode = "RGBA";
        }

        stbi_image_free(imageData);
    }

    void Application::SaveImageAsPPM(const std::string& inputPath, const std::string& outputPath)
    {
        std::vector<unsigned char> ppmData(imageInfo_.width * imageInfo_.height * 3);

        std::ifstream input(inputPath, std::ios::binary);
        if (!input.is_open())
        {
            std::cerr << "Could not open the input file." << std::endl;
            return;
        }

        std::string line;
        std::getline(input, line);
        std::string format = line;
        if (line != "P5" && line != "P6")
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

        std::vector<unsigned char> image;
        if (format == "P5")
        {
            image = std::vector<unsigned char>(width * height);
        }
        else if (format == "P6")
        {
            image = std::vector<unsigned char>(width * height * 3);
        }

        input.read(reinterpret_cast<char*>(image.data()), image.size());

        for (int i = 0; i < image.size(); i++)
        {
            if (format == "P5")
            {
                ppmData[i * 3] = image[i];
                ppmData[i * 3 + 1] = image[i];
                ppmData[i * 3 + 2] = image[i];
            }
            else if (format == "P6")
            {
                ppmData[i] = image[i];
            }
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