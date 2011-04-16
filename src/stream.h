// Music Room Interface
// --------------------
// stream.h - Wave streaming
// --------------------
// "©" Nmlgc, 2010-2011

#ifdef WIN32
#include <dsound.h>

#include <FXHash.h>
#include <FXStream.h>
#include <FXObject.h>
#include <FXThread.h>

class Streamer : public FXThread, FXObject
{
	friend class StreamerFront;

private:
	Streamer();

protected:
	bool	Active;

	// Streaming File Information
	// --------------------------
	TrackInfo* Track;
	TrackInfo* New;	// Track switch queue
	FXFile	CurFile;
	FXuint CurFNHash;
	ulong	Pos;
	// --------------------------

	// DirectSound API
	IDirectSound8* DS;	// Device
	WAVEFORMATEX Fmt;	// Sample rate

	// Vorbis Stuff
	OggVorbis_File SF;

	// Streaming Blocks
	IDirectSoundBuffer* SB;	// Sound Buffer
	ulong	Write;	// Current write cursor
	volatile bool StopReq;	// Set true to request thread stopping

	void StreamFrame_WAV(char* Buffer, const ulong& Size);
	void StreamFrame_OGG(char* Buffer, const ulong& Size);
	bool StreamFrame(const ulong& Offset, const ulong& Size); // Streaming Loop Function
	
	bool SwitchTrack_WAV(TrackInfo* NewTrack, FXString& NewFN);
	bool SwitchTrack_OGG(TrackInfo* NewTrack, FXString& NewFN);
	bool SwitchTrack();

public:
	static const int BlockSize = 0x4000;
	static const int BlockCount = 3;
	static const int BufferSize = BlockSize * BlockCount;
	
	virtual FXint run();	// Thread loop, runs streaming and track switching

	bool Init(void* xid);

	void SetVolume();

	void RequestTrackSwitch(TrackInfo* NewTrack);
	void Play();
	void Stop();	// Waits with returning until thread is done!

	void ClearBuffer();	// Resets whole buffer to zero. Only necessary on game switches.
	void CloseFile();
	void Exit();

	static Streamer& Inst()
	{
		static Streamer Instance;
		return Instance;
	}
};
#endif
