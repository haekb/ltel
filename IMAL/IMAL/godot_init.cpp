
#include <Godot.hpp>
#include <Node.hpp>
#include <Reference.hpp>

//#include "pch.h"
#include <string>

#include <direct.h>
#include <Windows.h>


using namespace godot;

typedef void* HSONG;
typedef void* HPLAYLIST;

// From SHOGO -> dwBoundaryFlags = ...
#define MUSIC_IMMEDIATE			0
#define MUSIC_NEXTMEASURE		1
#define MUSIC_NEXTSONG			2
#define MUSIC_QUEUE				3

struct MusicMgr {
    bool (*Init)(MusicMgr* pMusicMgr);
    void (*Term)();
    bool (*SetDataDirectory)(char* szDataDirectory);
    bool (*InitInstruments)(char* szDLSFile, char* szStyleFile);
    HSONG(*CreateSong)(char* szSong);
    HSONG(*GetSong)(char* szSong);
    void  (*DestroySong)(HSONG pSong);
    bool (*PlayBreak)(HSONG pSong, uint32_t dwBoundaryFlags);
    HPLAYLIST (*CreatePlayList)(char* szPlayList);
    HPLAYLIST (*GetPlayList)(char* szPlayList);
    bool (*AddSongToPlayList)(HPLAYLIST pPlayList, HSONG pSong);
    HSONG(*GetSongFromPlayList)(HPLAYLIST pPlayList); //, char/int? Returns 0!
    void (*RemovePlayList)(HPLAYLIST pPlayList);
    bool (*PlayList)(HPLAYLIST pPlayList, HSONG pTransition, bool bLoop, uint32_t dwBoundaryFlags);
    bool (*PlayMotif)(char* szMotif); // , bool bLoop) - ClientDE has a second option hmm...
    bool (*StopMotif)(char* szMotif);
    void (*DestroyAllSongs)();
    void (*Stop)(uint32_t dwBoundaryFlags);
    void (*Pause)(uint32_t dwBoundaryFlags);
    void (*Resume)();
    uint16_t (*GetVolume)();
    void (*SetVolume)(uint16_t nVolume);
    void (*CPrint)(char* pMsg, ...); // 99% sure
    // Maybe?
    bool bValid;
    bool bUnk; // Also true with valid.
    void* hWND;
    void* pUnk; // Not used?
    uint32_t nUnk; // 7?
};


typedef void f_MusicDLLSetup(MusicMgr* pMusicMgr);

void ConsolePrint(char* pMsg, ...)
{
    va_list list;
    char szMessage[256] = "";

    va_start(list, pMsg);

    vsnprintf(szMessage, 256, pMsg, list);

    va_end(list);

    godot::Godot::print("[CONSOLE PRINT]: {0}", szMessage);
}

class IMAL : public Node {
    GODOT_CLASS(IMAL, Node);
public:
    IMAL() { }

    /** `_init` must exist as it is called by Godot. */
    void _init() { }

    bool setup_ima(String sGameDirectory)
    {
        sGameDirectory += "ima.dll";

        char szBuffer[256] = "";
        auto pUnk = _getcwd(szBuffer, 256);

        //_chdir(sGameDirectory.alloc_c_string());

        // CWD is the project folder...
        HINSTANCE hIMA = LoadLibraryA("./bin/ima.dll");//sGameDirectory.alloc_c_string());

        if (!hIMA)
        {
            Godot::print("Could not locate ima.dll! Error: {0}", (uint32_t)GetLastError());
            return false;
        }

        f_MusicDLLSetup* pMusicDLLSetupFunc = (f_MusicDLLSetup*)GetProcAddress(hIMA, "MusicDLLSetup");

        if (!pMusicDLLSetupFunc)
        {
            Godot::print("Could not obtain Proc MusicDLLSetup!");
            return false;
        }

        MusicMgr* pMusicMgr = new MusicMgr();

        // Clear the sturct
        memset(pMusicMgr, 0, sizeof(MusicMgr));

        // Can't get hWND from Godot until next build, so let's try this!
        pMusicMgr->hWND = GetActiveWindow();
        // Make sure we're really not valid!
        pMusicMgr->bValid = false;
        // Setup our print function
        pMusicMgr->CPrint = ConsolePrint;

        pMusicDLLSetupFunc(pMusicMgr);

        if (!pMusicMgr->Init)
        {
            Godot::print("MusicDLLSetup failed to setup!");
            return false;
        }

        pMusicMgr->Init(pMusicMgr);

        if (!pMusicMgr->bValid)
        {
            Godot::print("Init failed to init!");
            return false;
        }

        // Setup our music dir
        bool bOk = false;

        bOk = pMusicMgr->SetDataDirectory((char*)"D:\\GOG Games\\Shogo - Mobile Armor Division - dev\\Music\\RedRiot");
        bOk = pMusicMgr->InitInstruments((char*)"redriot.dls", (char*)"redinit.sty");
        auto pPlayList = pMusicMgr->CreatePlayList((char*)"AmbientList");

        for (int i = 1; i < 13; i++)
        {
            std::string sSongName = "c" + std::to_string(i) + ".sec";
            auto pSong = pMusicMgr->CreateSong((char*) sSongName.c_str());
            bOk = pMusicMgr->AddSongToPlayList(pPlayList, pSong);
        }

        auto pTransition = pMusicMgr->CreateSong((char*)"stc.sec");

        bOk = pMusicMgr->PlayList(pPlayList, pTransition, true, MUSIC_IMMEDIATE);

        bool bHi = true;

        return true;

    }

    static void _register_methods() {

        register_method("setup_ima", &IMAL::setup_ima);

        /**
         * The line below is equivalent to the following GDScript export:
         *     export var _name = "IMAL"
         **/
        register_property<IMAL, String>("base/name", &IMAL::_name, String("IMAL"));

        /** Alternatively, with getter and setter methods: */
        register_property<IMAL, int>("base/value", &IMAL::set_value, &IMAL::get_value, 0);

        /** Registering a signal: **/
        // register_signal<IMAL>("signal_name");
        // register_signal<IMAL>("signal_name", "string_argument", GODOT_VARIANT_TYPE_STRING)
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

    godot::register_class<IMAL>();
}