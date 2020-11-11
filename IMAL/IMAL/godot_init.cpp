
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

MusicMgr* g_pMusicMgr = nullptr;
HPLAYLIST g_hPlaylist = nullptr;
HSONG g_hTransition = nullptr;

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

        g_pMusicMgr = new MusicMgr();

        // Clear the sturct
        memset(g_pMusicMgr, 0, sizeof(MusicMgr));

        // Can't get hWND from Godot until next build, so let's try this!
        g_pMusicMgr->hWND = GetActiveWindow();
        g_pMusicMgr->pUnk = (void*)0x1337; // Just in case it crashes due to access violation, I can track where this is used!
        // Make sure we're really not valid!
        g_pMusicMgr->bValid = false;
        // Setup our print function
        g_pMusicMgr->CPrint = ConsolePrint;

        pMusicDLLSetupFunc(g_pMusicMgr);

        if (!g_pMusicMgr->Init)
        {
            Godot::print("MusicDLLSetup failed to setup!");
            return false;
        }

        g_pMusicMgr->Init(g_pMusicMgr);

        if (!g_pMusicMgr->bValid)
        {
            Godot::print("Init failed to init!");
            return false;
        }

        // 100 sounds pretty bad
        g_pMusicMgr->SetVolume(75);

        // Setup our music dir
        bool bOk = false;
#if 0
#if 1
        std::string sDataDir = "D:\\GOG Games\\Blood 2 - dev\\Music\\A1";
        std::string sDLSFile = "Ancient.dls";
        std::string sStyleFile = "aoneinit.sty";
        std::string sFilePrefix = "c";
        std::string sTransitionFile = "stc.sec";
        int nFiles = 16;
#else
        // Defaults
        std::string sDataDir = "D:\\GOG Games\\Shogo - Mobile Armor Division - dev\\Music\\RedRiot";
        std::string sDLSFile = "redriot.dls";
        std::string sStyleFile = "redinit.sty";
        std::string sFilePrefix = "c";
        std::string sTransitionFile = "stc.sec";
        int nFiles = 13;
#endif

        bOk = g_pMusicMgr->SetDataDirectory((char*)sDataDir.c_str());
        bOk = g_pMusicMgr->InitInstruments((char*)sDLSFile.c_str(), (char*)sStyleFile.c_str());
        auto pPlayList = g_pMusicMgr->CreatePlayList((char*)"AmbientList");

        for (int i = 2; i < nFiles+1; i++)
        {
            std::string sSongName = sFilePrefix + std::to_string(i) + ".sec";
            auto pSong = g_pMusicMgr->CreateSong((char*) sSongName.c_str());
            bOk = g_pMusicMgr->AddSongToPlayList(pPlayList, pSong);
        }

        auto pTransition = g_pMusicMgr->CreateSong((char*)sTransitionFile.c_str());

        bOk = g_pMusicMgr->PlayList(pPlayList, pTransition, true, MUSIC_IMMEDIATE);
#endif
        return true;
    }

    void resume_playlist()
    {
        g_pMusicMgr->Resume();
    }

    void pause_playlist()
    {
        g_pMusicMgr->Pause(MUSIC_IMMEDIATE);
    }

    void stop_playlist()
    {
        g_pMusicMgr->Stop(MUSIC_IMMEDIATE);
    }

    void play_playlist()
    {
        if (!g_hTransition)
        {
            return;
        }

        g_pMusicMgr->PlayList(g_hPlaylist, g_hTransition, true, MUSIC_IMMEDIATE);
    }

    bool set_data_directory(String sDirectory)
    {
        godot::Godot::print("Stopping playback, and clearing memory.");

        g_pMusicMgr->Stop(MUSIC_IMMEDIATE);
        g_pMusicMgr->DestroyAllSongs();

        char* szDataDir = sDirectory.alloc_c_string();

        godot::Godot::print("Setting data directory {0}", szDataDir);

        return g_pMusicMgr->SetDataDirectory(szDataDir);
    }

    bool setup_instruments(String sDLSFile, String sStyleFile)
    {
        char* szDLS = sDLSFile.alloc_c_string();
        char* szStyle = sStyleFile.alloc_c_string();

        godot::Godot::print("Setting up instruments\nDLS File: {0}\nStyle File: {1}", szDLS, szStyle);

        return g_pMusicMgr->InitInstruments(szDLS, szStyle);
    }

    bool set_transition(String sTransition)
    {
        char* szTransition = sTransition.alloc_c_string();

        godot::Godot::print("Setting intro transition {0}", szTransition);

        auto pSong = g_pMusicMgr->CreateSong(szTransition);

        if (!pSong)
        {
            return false;
        }

        g_hTransition = pSong;
        return true;
    }

    bool create_playlist(Array aSongs)
    {
        // Remove any existing playlist
        if (g_hPlaylist)
        {
            g_pMusicMgr->RemovePlayList(g_hPlaylist);
            g_hPlaylist = nullptr;
        }

        g_hPlaylist = g_pMusicMgr->CreatePlayList((char*)"CoolTunes");

        bool bOk = false;

        for (int i = 0; i < aSongs.size(); i++)
        {
            String sSongname = aSongs[i];

            auto szSong = sSongname.alloc_c_string();
            godot::Godot::print("Adding {0}", szSong);

            auto pSong = g_pMusicMgr->CreateSong(szSong);
            bOk = g_pMusicMgr->AddSongToPlayList(g_hPlaylist, pSong);

            if (!bOk)
            {
                return false;
            }
        }

        return true;
    }

    static void _register_methods() {

        register_method("setup_ima", &IMAL::setup_ima);
        register_method("resume_playlist", &IMAL::resume_playlist);
        register_method("pause_playlist", &IMAL::pause_playlist);
        register_method("stop_playlist", &IMAL::stop_playlist);
        register_method("play_playlist", &IMAL::play_playlist);
        register_method("set_transition", &IMAL::set_transition);
        register_method("set_data_directory", &IMAL::set_data_directory);
        register_method("setup_instruments", &IMAL::setup_instruments);
        register_method("create_playlist", &IMAL::create_playlist);


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