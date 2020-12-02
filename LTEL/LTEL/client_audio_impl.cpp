#include "client.h"
#include "shared.h"
#include "helpers.h"

DBOOL impl_InitMusic(char* szMusicDLL)
{
	return TRUE;
}

DRESULT impl_InitSound(InitSoundInfo* pSoundInfo)
{
	// No sound settings yet!
	return DE_OK;
}

void impl_SetMusicVolume(short wVolume)
{
	godot::Godot::print("[impl_SetMusicVolume] Music volume is now {0}", wVolume);
	return;
}

void impl_SetSoundVolume(unsigned short nVolume)
{
	godot::Godot::print("[impl_SetSoundVolume] Sound volume is now {0}", nVolume);
	return;
}

DRESULT impl_SetReverbProperties(ReverbProperties* pReverbProperties)
{
	return DE_OK;
}

void impl_PauseSounds()
{
	godot::Godot::print("[impl_PauseSounds] Sounds are paused!");
}

void impl_ResumeSounds()
{
	godot::Godot::print("[impl_ResumeSounds] Sounds are unpaused!");
}

// Pause music.  Can be resumed...
DBOOL impl_PauseMusic()
{
	godot::Godot::print("[impl_PauseMusic] Music is paused!");
	return DE_OK;
}

// Resume music...
DBOOL impl_ResumeMusic()
{
	godot::Godot::print("[impl_ResumeMusic] Music is unpaused!");
	return DE_OK;
}

DRESULT impl_PlaySound(PlaySoundInfo* pPlaySoundInfo)
{
	//void* pData = malloc(4);
	pPlaySoundInfo->m_hSound = nullptr;// (HSOUNDDE)pData;
	return DE_OK;
}

void impl_KillSound(HSOUNDDE pSoundHandle)
{
	//free(pSoundHandle);
	return;
}

void impl_DestroyAllSongs()
{
	return;
}

DBOOL impl_PlayList(char* szPlayList, char* szTransition, DBOOL bLoop, DDWORD dwBoundaryFlags)
{
	godot::Godot::print("[impl_PlayList] Game wants to play {0} with transition {1}", szPlayList, szTransition);
	return DE_OK;
}

void LTELClient::InitAudioImpl()
{
	// Audio functionality
	InitMusic = impl_InitMusic;
	InitSound = impl_InitSound;
	SetMusicVolume = impl_SetMusicVolume;
	SetSoundVolume = impl_SetSoundVolume;
	SetReverbProperties = impl_SetReverbProperties;
	PauseSounds = impl_PauseSounds;
	ResumeSounds = impl_ResumeSounds;
	PauseMusic = impl_PauseMusic;
	ResumeMusic = impl_ResumeMusic;
	PlaySound = impl_PlaySound;
	KillSound = impl_KillSound;
	DestroyAllSongs = impl_DestroyAllSongs;
	PlayList = impl_PlayList;
}