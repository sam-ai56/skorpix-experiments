#include <iostream>
#include <chrono>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <IconsCodicons.h>


const ImVec2 WINDOW_SIZE = ImVec2(600, 28);

bool is_pauseLoop = false;
int frames_to_skip = 0;
int skipped_frames = 0;

void pauseLoop(bool frames = 0)
{
    is_pauseLoop = true;
    frames_to_skip = frames;
}

void resumeLoop()
{
    is_pauseLoop = false;
    frames_to_skip = 0;
    skipped_frames = 0;
}

int main(int argc, char** argv) {
    Display* display;
    Window window;
    Screen* screen;
    int screenId;
    XEvent ev;

    // Open the display
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        std::cout << "Could not open display\n";
        return 1;
    }
    screen = DefaultScreenOfDisplay(display);
    screenId = DefaultScreen(display);


    // Check GLX version
    GLint majorGLX, minorGLX = 0;
    glXQueryVersion(display, &majorGLX, &minorGLX);
    if (majorGLX <= 1 && minorGLX < 2) {
        std::cout << "GLX 1.2 or greater is required.\n";
        XCloseDisplay(display);
        return 1;
    }
    else {
        std::cout << "GLX version: " << majorGLX << "." << minorGLX << '\n';
    }

    // GLX, create XVisualInfo, this is the minimum visuals we want
    GLint glxAttribs[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   8,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES,        0,
        None
    };
    XVisualInfo* visual = glXChooseVisual(display, screenId, glxAttribs);

    if (visual == 0) {
        std::cout << "Could not create correct visual window.\n";
        XCloseDisplay(display);
        return 1;
    }

    // Open the window
    XSetWindowAttributes windowAttribs;
    windowAttribs.border_pixel = BlackPixel(display, screenId);
    windowAttribs.background_pixel = WhitePixel(display, screenId);
    windowAttribs.override_redirect = True;
    windowAttribs.colormap = XCreateColormap(display, RootWindow(display, screenId), visual->visual, AllocNone);
    windowAttribs.event_mask = KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|EnterWindowMask|LeaveWindowMask|StructureNotifyMask;
    window = XCreateWindow(display, RootWindow(display, screenId), 0, 0, WINDOW_SIZE.x, WINDOW_SIZE.y, 0, visual->depth, InputOutput, visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

    // Create GLX OpenGL context
    GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
    glXMakeCurrent(display, window, context);

    std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // Show the window
    XClearWindow(display, window);
    XMapRaised(display, window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // disable config creation
    io.IniFilename = NULL;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    io.DisplaySize = WINDOW_SIZE;
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 1));
    //ImGui::StyleColorsClassic();

    ImVec4 primary_color = ImVec4(0.898f, 0.102f, 0.18f, 1.0f); // #e51a2e


    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Backgrounds
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);

    // Buttons
    style.Colors[ImGuiCol_Button] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_ButtonHovered] = primary_color;
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.980f, 0.204f, 0.294f, 1.0f);

    // Headers
    style.Colors[ImGuiCol_Header] = ImVec4(0.4f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.6f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.0f, 0.0f, 1.0f);


    // Setup Platform/Renderer backends
    ImGui_ImplOpenGL3_Init();

    //io.Fonts->AddFontDefault();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("./fonts/Inter/static/Inter-Light.ttf", 17.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    float baseFontSize = 24.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    static const ImWchar icons_ranges[] = { ICON_MIN_CI, ICON_MAX_16_CI, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF("./fonts/vscode-codicons/dist/codicon.ttf", iconFontSize, &icons_config, icons_ranges);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    int frame_count = 0;
    auto t_last = std::chrono::high_resolution_clock::now();

    bool done = false;
    while (!done) {
        if (XPending(display) > 0)
        {
            XNextEvent(display, &ev);
            if (ev.type == ConfigureNotify)
            {
                resumeLoop();
                io.DisplaySize = ImVec2(ev.xconfigure.width, ev.xconfigure.height);
                pauseLoop(1);
            }
            // the mouse moves in the window area
            if (ev.type == MotionNotify)
            {
                io.MousePos = ImVec2(ev.xmotion.x, ev.xmotion.y);
            }
            // the mouse is out of the window area
            if (ev.type == LeaveNotify)
            {
                io.MousePos = ImVec2(-1, -1);
                pauseLoop(1);
            }
            // the mouse is in the window area
            if (ev.type == EnterNotify)
            {
                resumeLoop();
            }
            // button press event
            if (ev.type == ButtonPress)
            {
                io.MousePos = ImVec2(ev.xbutton.x, ev.xbutton.y);
                std::cout << "Button Pressed " << ev.xbutton.button << "\n";
                // if the mouse wheel is scrolled up
                if (ev.xbutton.button == 4)
                {
                    io.MouseWheel += 1.0;
                }
                // if the mouse wheel is scrolled down
                else if (ev.xbutton.button == 5)
                {
                    io.MouseWheel -= 1.0;
                }
                else
                {
                    io.MouseDown[ev.xbutton.button - 1] = true;
                }
            }
            if (ev.type == ButtonRelease)
            {
                io.MousePos = ImVec2(ev.xbutton.x, ev.xbutton.y);
                std::cout << "Button Released " << ev.xbutton.button << "\n";
                io.MouseDown[ev.xbutton.button - 1] = false;
            }
            // if (ev.type == KeyPress)
            // {
            //     io.KeysDown[ev.xkey.keycode] = true;
            //     std::cout << "Key Pressed\n";
            //     std::cout << ev.xkey.keycode << "\n";
            // }
            continue;
        }

        if (is_pauseLoop)
        {
            if (skipped_frames == frames_to_skip)
            {
                continue;
            }
            skipped_frames++;
        }

        auto t_current = std::chrono::high_resolution_clock::now();
        double t_delta = std::chrono::duration<double, std::milli>(t_current - t_last).count() / 1000.0;
        io.DeltaTime = t_delta;
        t_last = t_current;

        double fps = 1.0 / t_delta;
        frame_count++;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("Hello, world!", NULL,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus
            );

            //style.Colors[ImGuiCol_ButtonHovered] = primary_color;
            if (ImGui::Button(ICON_CI_CHROME_CLOSE, ImVec2(34, 28)))
            {
                // stop loop (close window)
                break;
            }

            ImGui::SameLine(0, 0);

            //style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3, 0.3, 0.3, 1);

            if (ImGui::Button(ICON_CI_CHROME_MAXIMIZE, ImVec2(34, 28)))
            {
                std::cout << "Maximize button clicked!\n";
            }

            ImGui::SameLine(0, 0);

            if (ImGui::Button(ICON_CI_CHROME_MINIMIZE, ImVec2(34, 28)))
            {
                std::cout << "Minimize button clicked!\n";
            }

            ImGui::SameLine(0, 0);

            const char* title = u8"Hello World!";

            ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - ImGui::CalcTextSize(title).x * 0.5f);
            ImGui::SetCursorPosY(ImGui::GetWindowSize().y * 0.3f - ImGui::CalcTextSize(title).y * 0.5f);

            ImGui::Text(title);

            // text on end
            ImGui::SameLine(ImGui::GetWindowSize().x - ImGui::CalcTextSize("(%.1f FPS) ").x);
            ImGui::Text("(%.1f FPS)", io.Framerate);

            ImGui::End();
        }

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glXSwapBuffers(display, window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    // Cleanup GLX
    glXDestroyContext(display, context);

    // Cleanup X11
    XFree(visual);
    XFreeColormap(display, windowAttribs.colormap);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}