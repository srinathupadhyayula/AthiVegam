#include "AthiVegam/Engine.h"
#include "AthiVegam/Log.h"
#include "sdl2/SDL.h"

//Test includes
#include "AthiVegam/Graphics/Mesh.h"
#include "AthiVegam/Graphics/Shader.h"

#include "Athivegam/Input/Mouse.h"
#include "Athivegam/Input/Keyboard.h"
#include "Athivegam/Input/Controller.h"

#include <string>

namespace AthiVegam 
{
    Engine::Engine() 
        : m_isRunning(false) 
        , m_isInitialized(false)
    { 
        
    }

    Engine::~Engine() 
    { 
        Shutdown();
    }

    bool Engine::Initialize() 
    {
        bool ret = false;

        VEGAM_ASSERT(!m_isInitialized, "Attempting to call Engine::Initialize() more than once!");
        if (!m_isInitialized)
        {
            m_logManager.Initialize();
            GetInfo();

            if (SDL_Init(SDL_INIT_EVERYTHING))
            {
                VEGAM_ERROR("Error initializing SDL2: {}", SDL_GetError());
            }
            else
            {
                SDL_version version;
                SDL_VERSION(&version);
                VEGAM_INFO("SDL {}.{}.{}"
                    , (int32_t)version.major
                    , (int32_t)version.minor
                    , (int32_t)version.patch);

                if (m_window.Create())
                {
                    // Initialize Managers
                    m_renderManager.Initialize();

                    // Initialize Input
                    Input::Mouse::Initialize();
                    Input::Keyboard::Initialize();

                    ret = true;
                }
            }

            m_isRunning = ret;
            m_isInitialized = ret;

            if (!ret)
            {
                VEGAM_ERROR("Engine failed to initialize!");
                Shutdown();
            }
        }

        return ret;
    }

    void Engine::Shutdown() 
    {
        m_isInitialized = false;
        m_isRunning = false;

        // Shutdown managers
        m_renderManager.Shutdown();
        m_logManager.Shutdown();

        // Shutdown SDL
        m_window.Shutdown();
        SDL_Quit();
    }

    void Engine::Run() 
    {
        if (Initialize())
        {
            {
                // Tesh Mesh
                float verts[]
                {
                     0.5f,    0.5f,  0.0f,
                     0.5f,   -0.5f,  0.0f,
                    -0.5f,   -0.5f,  0.0f,
                    -0.5f,    0.5f,  0.0f
                };
                uint32_t elements[]
                {
                    0, 1, 3,
                    1, 2, 3
                };
                //auto mesh = std::make_shared<Graphics::Mesh>(&verts[0], 3, 3);
                auto mesh = std::make_shared<Graphics::Mesh>(&verts[0], 4, 3, &elements[0], 6);

                // Test Shader
                const char* vertShader = R"(
                    #version 410 core
                    layout (location = 0) in vec3 position;
                    out vec3 vpos;
                    uniform vec2 offset = vec2(0.5f);
                    void main()
                    {
                        vpos = position + vec3(offset, 0);
                        gl_Position = vec4(position, 1.0f);
                    }
                )";
                const char* fragShader = R"(
                    #version 410 core
                    out vec4 outColor;
                    in vec3 vpos;
                    uniform vec3 color = vec3(0.0f);
                    void main()
                    {
                        outColor = vec4(color, 1.0);
                        outColor = vec4(vpos, 1.0);
                    }
                )";
                auto shader = std::make_shared<Graphics::Shader>(vertShader, fragShader);
                shader->SetUniformFloat3("color", 1.f, 0.f, 0.f);

                // Core Loop
                while (m_isRunning)
                {
                    m_window.PumpEvents();
                    //VEGAM_TRACE("X: {}, Y: {}, {} {} {} {} {}"
                    //    , Input::Mouse::X()
                    //    , Input::Mouse::Y()
                    //    , Input::Mouse::Button(Input::MouseButton::MOUSE_LEFT)
                    //    , Input::Mouse::Button(Input::MouseButton::MOUSE_MIDDLE)
                    //    , Input::Mouse::Button(Input::MouseButton::MOUSE_RIGHT)
                    //    , Input::Mouse::Button(Input::MouseButton::MOUSE_X1)
                    //    , Input::Mouse::Button(Input::MouseButton::MOUSE_X2));

                    //if (Input::Mouse::Button(Input::MouseButton::AV_MOUSE_LEFT))
                    //{
                    //    VEGAM_WARN("LeftButtonIsDown");
                    //}
                    //if (Input::Mouse::ButtonDown(Input::MouseButton::AV_MOUSE_LEFT))
                    //{
                    //    VEGAM_INFO("LeftButtonPressedDown");
                    //}
                    //if (Input::Mouse::ButtonUp(Input::MouseButton::AV_MOUSE_LEFT))
                    //{
                    //    VEGAM_ERROR("LeftButtonReleased");
                    //}
                    //int windowW = 0;
                    //int windowH = 0;
                    //GetWindow().GetSize(windowW, windowH);
                    //VEGAM_TRACE("X: {}, Y: {}", windowW, windowH);

                    //float xNorm = (float)Input::Mouse::X() / (float)windowW;
                    //float yNorm = (float)(windowH - Input::Mouse::Y()) / (float)windowH;
                    //float xNorm = (float)Input::Mouse::DX() / 100.f;
                    //float yNorm = (float)Input::Mouse::DY() / 100.f;
                    //
                    //if (Input::Keyboard::KeyDown(Input::KeyCode::AV_KEY_R))
                    //{
                    //    xNorm += 1.f;
                    //    VEGAM_TRACE("Key R Pressed!");
                    //}
                    //if (Input::Keyboard::KeyUp(Input::KeyCode::AV_KEY_R))
                    //{
                    //    xNorm += 1.f;
                    //    VEGAM_TRACE("Key R Released!");
                    //}
                    //if (Input::Keyboard::Key(Input::KeyCode::AV_KEY_R))
                    //{
                    //    xNorm += 1.f;
                    //    VEGAM_TRACE("Key R Down!");
                    //}
                    //
                    //shader->SetUniformFloat2("offset", xNorm, yNorm);

                    /*VEGAM_TRACE("\nControler Axes\n_______________\nLSH:{}\tLSV:{}\tRSH:{}\tRSV:{}\n_______________\nButton States\n_______________\nA:{}\tB:{}\tX:{}\tY:{}\n_______________\n"
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::LEFT_STICK_HORIZONTAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::LEFT_STICK_VERTICAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::RIGHT_STICK_HORIZONTAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::RIGHT_STICK_VERTICAL)
                        , Input::Controller::GetButton(0, Input::ControllerButton::A)
                        , Input::Controller::GetButton(0, Input::ControllerButton::B)
                        , Input::Controller::GetButton(0, Input::ControllerButton::X)
                        , Input::Controller::GetButton(0, Input::ControllerButton::Y));
                    */
                    /*VEGAM_WARN("\nControler Axes\n_______________\nLSH:{}\tLSV:{}\tRSH:{}\tRSV:{}\n_______________\n"
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::LEFT_STICK_HORIZONTAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::LEFT_STICK_VERTICAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::RIGHT_STICK_HORIZONTAL)
                        , Input::Controller::GetAxis(0, Input::ControllerAxis::RIGHT_STICK_VERTICAL)
                        );
                    */
                    VEGAM_INFO("RawLSH:{}\tDeadzonedLSH:{}", Input::Controller::GetAxisRaw(0, Input::ControllerAxis::LEFT_STICK_HORIZONTAL), Input::Controller::GetAxis(0, Input::ControllerAxis::LEFT_STICK_HORIZONTAL))

                    m_window.BeginRender();

                    auto rc = std::make_unique<Graphics::RenderCommands::RenderMesh>(mesh, shader);
                    m_renderManager.Submit(std::move(rc));
                    m_renderManager.Flush();

                    m_window.EndRender();
                }
            }
            Shutdown();
        }
    }

    void Engine::Quit() 
    {
        m_isRunning = false; 
    }

    void Engine::GetInfo() 
    {
        VEGAM_INFO("AthiVegam v{}.{}", 0, 1);
    #ifdef AV_CONFIG_DEBUG
        VEGAM_DEBUG("Configuration : DEBUG");
    #endif //  AV_CONFIG_DEBUG
    #ifdef AV_CONFIG_RELEASE
        VEGAM_DEBUG("Configuration : RELEASE");
    #endif // AV_CONFIG_RELEASE
    #ifdef AV_PLATFORM_WINDOWS
        VEGAM_WARN("Platform : WINDOWS");
    #endif // AV_PLATFORM_WINDOWS
    #ifdef AV_PLATFORM_LINUX
        VEGAM_WARN"Platform : LINUX");
    #endif // AV_PLATFORM_LINUX
    #ifdef AV_PLATFORM_MAC
        VEGAM_WARN"Platform : MAC");
    #endif // AV_PLATFORM_MAC
    }
} // namespace AthiVegam
