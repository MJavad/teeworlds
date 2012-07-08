/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_SOUND_H
#define ENGINE_CLIENT_SOUND_H

#include <engine/sound.h>

class CSound : public IEngineSound
{
	int m_SoundEnabled;

public:
	IEngineGraphics *m_pGraphics;
	IStorage *m_pStorage;
	IDemoPlayer *m_pDemo;

	virtual int Init();

	int Update();
	int Shutdown();
	int AllocID();

	static void RateConvert(int SampleID);

	// TODO: Refactor: clean this mess up
	static IOHANDLE ms_File;
	static int ReadData(void *pBuffer, int Size);

	virtual bool IsSoundEnabled() { return m_SoundEnabled != 0; }

	virtual int LoadWV(const char *pFilename);

	virtual void SetListenerPos(float x, float y);
	virtual void SetChannel(int ChannelID, float Vol, float Pan);

	int Play(int ChannelID, int SampleID, int Flags, float x, float y);
	virtual int PlayAt(int ChannelID, int SampleID, int Flags, float x, float y);
	virtual int Play(int ChannelID, int SampleID, int Flags);
	virtual void Stop(int SampleID);
	virtual void StopAll();

	virtual int AddWaveToStream(const char *pWave);
	virtual int GetWaveFrameSize() { return m_MusicFrameCount * 4; }
	virtual int GetWaveBufferSpace() { return MUSICTMPBUFFERSIZE - m_LuaTmpBufferIn; }

	virtual void MixHook(short *pFinalOut, unsigned Frames);
};

#endif
