#include <GlobalConstants.hpp>
#include <Godot.hpp>
#include <Reference.hpp>
#include <Node.hpp>

#include <map>

#include "framework.h"
#include <iostream>

#include <Input.hpp>
#include <VisualServer.hpp>

#include "client.h"
#include "LT1//AppHeaders/cpp_clientshell_de.h"

#include "ScancodeToVK.h"

#include "RiotCommandIDs.h"

#include "model_helper.h"


using namespace godot;

typedef int f_GetClientShellVersion();
typedef void f_GetClientShellFunctions(CreateClientShellFn* pCreate, DeleteClientShellFn* pDelete);

static LTELClient* g_pClient = nullptr;

//#define WAIT_FOR_DEBUGGER 

#include <Engine.hpp>

class LTEL : public Node {
    GODOT_CLASS(LTEL, Node);
public:
    CClientShellDE* m_pGameClientShell;

    LTEL() 
    {
        m_pGameClientShell = nullptr;
    }

    

    /** `_init` must exist as it is called by Godot. */
    void _init() { }

    bool initialize_cshell(godot::String sGameDataDir)
    {
        // Lock the framerate for accuracy with LT1 game code 
        // (Game code is always targetting 60fps because ugh.)
        Engine::get_singleton()->set_target_fps(60);


#ifdef WAIT_FOR_DEBUGGER
        while (!::IsDebuggerPresent())
        {
            ::Sleep(100); // to avoid 100% CPU load
        }
#endif

        //
        // Setup SEH translator
        // Be sure to enable "Yes with SEH Exceptions (/EHa)" in C++ / Code Generation;
        _set_se_translator([](unsigned int u, EXCEPTION_POINTERS* pExp) {
            std::string error = "SE Exception: ";
            switch (u) {
            case 0xC0000005:
                error += "Access Violation";
                break;
            default:
                char result[11];
                sprintf_s(result, 11, "0x%08X", u);
                error += result;
            };
            throw std::exception(error.c_str());
            });
        //

        // CWD is the project folder...
        HINSTANCE hClientShell = LoadLibraryA("./bin/CShell.dll");

        if (!hClientShell)
        {
            Godot::print("Could not locate CShell.dll!");
            return false;
        }

        HINSTANCE hCRes = LoadLibraryA("./bin/CRes.dll");

        if (!hCRes)
        {
            Godot::print("Could not locate CRes.dll!");
            return false;
        }

        // Initial test, print out the version!
        f_GetClientShellVersion* pGetVersion = (f_GetClientShellVersion*)GetProcAddress(hClientShell, "GetClientShellVersion");

        if (!pGetVersion)
        {
            Godot::print("Could not obtain Proc GetClientShellVersion!");
            return false;
        }

        // Print the version!
        int nVersion = pGetVersion();
        Godot::print("CShell.dll version: {0}", nVersion);

        if (nVersion != 3)
        {
            Godot::print("CShell interface version is not 3!");
            return false;
        }

        // Okay now the real fun begins,
        // we'll need to kick off our client impl to CShell.dll

        f_GetClientShellFunctions* pClientShellInitFunc = (f_GetClientShellFunctions*)GetProcAddress(hClientShell, "GetClientShellFunctions");

        if (!pClientShellInitFunc)
        {
            Godot::print("Could not obtain Proc GetClientShellFunctions!");
            return false;
        }

        //
        int pnCreate = 0;
        int pnDelete = 0;

        pClientShellInitFunc((CreateClientShellFn*)&pnCreate, (DeleteClientShellFn*)&pnDelete);

        if (!pnCreate)
        {
            Godot::print("Could not obtain CreateClientShellFn!");

            return false;
        }

        // Create our ClientDE instance
        g_pClient = new LTELClient(this, hCRes);

        // We'll want to run CreateClientShellFn, to get the game's GameClientShell instance
        CreateClientShellFn pCreate = (CreateClientShellFn)pnCreate;
        auto pClient = pCreate(g_pClient);
        m_pGameClientShell = (CClientShellDE*)pClient;

        g_pClient->m_pClientShell = m_pGameClientShell;

        if (!m_pGameClientShell)
        {
            Godot::print("Could not retrieve GameClientShell!");
            return false;
        }

        DGUID AppGUID = { 0 };

        g_pClient->m_sGameDataDir = sGameDataDir.alloc_c_string();
        
        // Setup any autoexec.cfg commands here (Until we actually load in the autoexec.cfg!)
        g_pClient->RunConsoleString((char*)"MouseLook 1.0");
        //g_pClient->RunConsoleString((char*)"PlayerDims 1.0");

        // Kick off OnEngineInit
        try {
            // We should really populate RMode soon...
            auto hResult = m_pGameClientShell->OnEngineInitialized(nullptr, &AppGUID);
            Godot::print("OnEngineInit returned {0}", (int)hResult);
        }
        catch (const std::exception& e)
        {
            Godot::print("[OnEngineInitalized] Failed with exception: {0}", e.what());
        }

        // If we want to manually control drawing, here's how we'd do it!
        auto pVS = godot::VisualServer::get_singleton();
        //pVS->call("set_render_loop_enabled", false);

        

        Godot::print("Done!");
        return true;
    }
    
    void game_update(float fDelta)
    {
        static bool bFirstUpdate = true;

        if (bFirstUpdate)
        {

            auto pVS = godot::VisualServer::get_singleton();
            //pVS->call("set_render_loop_enabled", true);
            bFirstUpdate = false;
        }

        // This is for GetFrameTime() impl
        g_pClient->m_fFrametime = fDelta;

        if (g_pClient->m_pLTELServer)
        {
            g_pClient->m_pLTELServer->HandleMessageQueue();
        }

        // Run our update functions
        try {
            m_pGameClientShell->PreUpdate();

            m_pGameClientShell->Update();

            m_pGameClientShell->PostUpdate();
        }
        catch (const std::exception& e)
        {
            Godot::print("[game_update] Failed with client exception: {0}", e.what());
        }

        //try {
            if (g_pClient->m_pLTELServer && g_pClient->m_pLTELServer->m_pServerShell)
            {
                // Object update
                g_pClient->m_pLTELServer->Update(fDelta);

                g_pClient->m_pLTELServer->m_pServerShell->Update(fDelta);
            }
            /*}
    
        catch (const std::exception& e)
        {
            Godot::print("[game_update] Failed with server exception: {0}", e.what());
        }
        */

        // Clear mouse motion for this frame!
        g_pClient->m_vRelativeMouse = godot::Vector2();
    }

    void on_key_input(int nScancode, bool bPressed)
    {
        int nVK = 0;

        try {
            nVK = scancode_to_vk.at(nScancode);
        }
        catch (const std::exception& e)
        {
            Godot::print("[on_key_input] Failed with scancode_to_vk failure: {0}", e.what());
            return;
        }

        static std::map<int, int> mInputList = {
            { VK_CONTROL, COMMAND_ID_FIRING },
            { VK_SPACE, COMMAND_ID_JUMP },
            { 0x57, COMMAND_ID_FORWARD },           // W
            { 0x53, COMMAND_ID_REVERSE },           // S
            { 0x41, COMMAND_ID_STRAFE_LEFT },       // A
            { 0x44, COMMAND_ID_STRAFE_RIGHT },      // D
            { 0x31, COMMAND_ID_NEXT_WEAPON },       // 1
            { 0x32, COMMAND_ID_PREV_WEAPON },       // 2
            { 0x43, COMMAND_ID_DUCK },              // C
            { 0x59, COMMAND_ID_MESSAGE },           // Y
        };

        int nCommand = -1;
        if (mInputList.find(nVK) != mInputList.end())
        {
            nCommand = mInputList[nVK];
        }

        CClientShellDE* pClientShell = (CClientShellDE*)g_pClient->GetClientShell();

        if (bPressed)
        {
            m_pGameClientShell->OnKeyDown(nVK, 0);

            if (nCommand != -1 && g_pClient->m_bAllowInput)
            {
                g_pClient->SetCommandOn(nCommand);
                pClientShell->OnCommandOn(nCommand);
            }

            return;
        }


        m_pGameClientShell->OnKeyUp(nVK);

        if (nCommand != -1 && g_pClient->m_bAllowInput)
        {
            g_pClient->SetCommandOff(nCommand);
            pClientShell->OnCommandOff(nCommand);
        }
    }

    void on_mouse_motion(Vector2 vRelative)
    {
        if (!g_pClient->m_bAllowInput)
        {
            g_pClient->m_vRelativeMouse = godot::Vector2(0, 0);
            return;
        }

        //Godot::print("[on_mouse_motion] {0}", vRelative);
        g_pClient->m_vRelativeMouse = -vRelative;
    }

    void test_void_method() {
        Godot::print("This is test");
    }

    Variant method(Variant arg) {
        Variant ret;
        ret = arg;
        return ret;
    }

    static void _register_methods() {
        // Test functions
        register_method("method", &LTEL::method);
        register_method("test_void_method", &LTEL::test_void_method);

        // LTEL functions
        register_method("init_cshell", &LTEL::initialize_cshell);
        register_method("game_update", &LTEL::game_update);
        register_method("on_key_input", &LTEL::on_key_input);
        register_method("on_mouse_motion", &LTEL::on_mouse_motion);

        /**
         * The line below is equivalent to the following GDScript export:
         *     export var _name = "LTEL"
         **/
        register_property<LTEL, String>("base/name", &LTEL::_name, String("LTEL"));

        /** Alternatively, with getter and setter methods: */
        register_property<LTEL, int>("base/value", &LTEL::set_value, &LTEL::get_value, 0);

        /** Registering a signal: **/
        // register_signal<LTEL>("signal_name");
        // register_signal<LTEL>("signal_name", "string_argument", GODOT_VARIANT_TYPE_STRING)
    }

    String _name;
    int _value;

    void set_value(int p_value) {
        _value = p_value;
    }

    int get_value() const {
        return _value;
    }
};

/** GDNative Initialize **/
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options * o) {
    godot::Godot::gdnative_init(o);
}

/** GDNative Terminate **/
extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options * o) {
    godot::Godot::gdnative_terminate(o);
}

/** NativeScript Initialize **/
extern "C" void GDN_EXPORT godot_nativescript_init(void* handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<LTEL>();
    godot::register_class<ModelHelper>();
}