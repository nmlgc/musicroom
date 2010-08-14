// Touhou Project BGM Extractor
// ----------------------------
// stream.h - Wave streaming
// ----------------------------
// "©" Nameless, 2010

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>

static CRITICAL_SECTION WaveCS;

class Streamer
{
private:
	Streamer();

protected:
	bool	Active;

	// Streaming File Information
	// --------------------------
	TrackInfo* Track;
	FILE*	BGMFile;
	ulong	Pos;

	float VolLog; // Volume in range [0.0 ; 1.0]

	static const int ReadBufferSize = 16384;
	char ReadBuffer[Streamer::ReadBufferSize];
	// --------------------------

	// waveOut API
	HWAVEOUT Dev;		// waveOut Device
	WAVEFORMATEX Fmt;	// Sample rate

	// Vorbis Stuff
	OggVorbis_File SF;

	// Streaming Blocks
	WAVEHDR* Block;	// Audio blocks
	uint CurBlock;	// Block pointer
	volatile uint FreeBlockCount; // volatile = can be modified from different threads

	void UnprepareBlocks();
	void CreateBlocks();
	void ClearBlocks();

	void StreamFrame_WAV(); // Streaming Loop Function
	void StreamFrame_OGG();
	bool SwitchTrack_WAV(TrackInfo* NewTrack);
	bool SwitchTrack_OGG(TrackInfo* NewTrack);

public:
	static const int BlockSize = Streamer::ReadBufferSize;
	static const int BlockCount = 5;

	bool Init();

	bool WriteAudio(char* Raw, int Size);	// returns false if audio data has to be cached because all buffers are full

	void StreamFrame(); // Streaming Loop Function
	bool SwitchTrack(TrackInfo* NewTrack);

	void Stop();
	void CloseFile();

	void Exit();

	static Streamer& Inst()
	{
		static Streamer Instance;
		return Instance;
	}
};
#endif
