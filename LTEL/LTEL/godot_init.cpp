
#include <Godot.hpp>
#include <Reference.hpp>

#include "framework.h"
#include <iostream>

#include "LT1/AppHeaders/client_de.h"
//#include "pch.h"

using namespace godot;

typedef int f_GetClientShellVersion();
typedef void f_GetClientShellFunctions(CreateClientShellFn* pCreate, DeleteClientShellFn* pDelete);

class LTEL : public Reference {
    GODOT_CLASS(LTEL, Reference);
public:
    LTEL() { }

    /** `_init` must exist as it is called by Godot. */
    void _init() { }

    void initialize_cshell()
    {
        // CWD is the project folder...
        HINSTANCE hClientShell = LoadLibraryA("./bin/CShell.dll");

        TCHAR NPath[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, NPath);

        Godot::print(NPath);

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
        }
        else
        {
            int nVersion = pGetVersion();
            Godot::print("CShell.dll version: {0}", nVersion);
        }

        //

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