
#include <Godot.hpp>
#include <Reference.hpp>
#include <Node.hpp>

#include "framework.h"
#include <iostream>

#include "client.h"
#include "LT1//AppHeaders/cpp_clientshell_de.h"
//#include "pch.h"

using namespace godot;

typedef int f_GetClientShellVersion();
typedef void f_GetClientShellFunctions(CreateClientShellFn* pCreate, DeleteClientShellFn* pDelete);

static LTELClient* g_pClient = nullptr;

//#define WAIT_FOR_DEBUGGER 

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

    void initialize_cshell()
    {
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
            return;
        }

        HINSTANCE hCRes = LoadLibraryA("./bin/CRes.dll");

        if (!hCRes)
        {
            Godot::print("Could not locate CRes.dll!");
            return;
        }
        /*
        f_GetClientShellFunctions* pClientShellInitFunc = (f_GetClientShellFunctions*)GetProcAddress(hClientShell, "GetClientShellFunctions");

        CreateClientShellFn* pCreate = nullptr;
        DeleteClientShellFn* pDelete = nullptr;

        pClientShellInitFunc(pCreate, pDelete);
        */

        f_GetClientShellVersion* pGetVersion = (f_GetClientShellVersion*)GetProcAddress(hClientShell, "GetClientShellVersion");

        if (!pGetVersion)
        {
            Godot::print("Could not obtain Proc GetClientShellVersion!");
            return;
        }

        // Print the version!
        int nVersion = pGetVersion();
        Godot::print("CShell.dll version: {0}", nVersion);

        // Okay now the real fun begins,
        // we'll need to kick off our client impl to CShell.dll

        f_GetClientShellFunctions* pClientShellInitFunc = (f_GetClientShellFunctions*)GetProcAddress(hClientShell, "GetClientShellFunctions");

        if (!pClientShellInitFunc)
        {
            Godot::print("Could not obtain Proc GetClientShellFunctions!");
            return;
        }

        //
        int pnCreate = 0;
        int pnDelete = 0;

        pClientShellInitFunc((CreateClientShellFn*)&pnCreate, (DeleteClientShellFn*)&pnDelete);

        if (!pnCreate)
        {
            Godot::print("Could not obtain CreateClientShellFn!");

            return;
        }

        Godot::print("We have pnCreate!");

        g_pClient = new LTELClient(this, hCRes);

        CreateClientShellFn pCreate = (CreateClientShellFn)pnCreate;
        m_pGameClientShell = (CClientShellDE*)pCreate(g_pClient);

        if (!m_pGameClientShell)
        {
            Godot::print("Could not retrieve GameClientShell!");
            return;
        }

        DGUID AppGUID = { 0 };

        try {

            auto hResult = m_pGameClientShell->OnEngineInitialized(nullptr, &AppGUID);
            Godot::print("OnEngineInit returned {0}", (int)hResult);
        }
        catch (const std::exception& e)
        {
            Godot::print("[OnEngineInitalized] Failed with exception: {0}", e.what());
        }

        Godot::print("Done!");
    }

    void game_update() 
    {
        try {
            m_pGameClientShell->PreUpdate();

            m_pGameClientShell->Update();

            m_pGameClientShell->PostUpdate();
        }
        catch (const std::exception& e)
        {
            Godot::print("[game_update] Failed with exception: {0}", e.what());
        }

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
        register_method("method", &LTEL::method);
        register_method("test_void_method", &LTEL::test_void_method);


        register_method("init_cshell", &LTEL::initialize_cshell);
        register_method("game_update", &LTEL::game_update);


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
}