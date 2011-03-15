// Music Room Interface
// --------------------
// extract.h - Extracting and encoding functions
// --------------------
// "©" Nmlgc, 2010

extern const ushort WAV_HEADER_SIZE;

// Fade algorithms
class FadeAlg
{
public:
	FXString	Name;

	virtual short*	Eval(short* f, long& c, long& Len) = 0;
	virtual ~FadeAlg()	{}
};

// Extraction state variables
struct Extract_Vals
{
	FXFile In;
	FXFile Out;
	FXString DisplayFN;

	// All of these are absolute!
	ulong	ts_data;	// digital track start
	ulong	ts_ext;		// extraction start (= <ts_data>, unless silence is removed)
	ulong	tl;			// loop start
	ulong	te;			// loop position

	long	Len;	// Total track length, incl. loops and fades

	FadeAlg*	FA;
	long	FadeStart;
	long	FadeBytes;
	long	f;	// Fade progression

	char*	Buf;	// Temporary extraction buffer
	bool	TagEngine;	// Should the tag engine tag this one?

	// Thread
	volatile bool* StopReq;	// points to Encoder::StopReq
	volatile FXulong d;		// Progress
	volatile uint*	Ret;	// points to Extractor::Ret

	Extract_Vals();
	Extract_Vals(TrackInfo* TI, const bool& Fmt);
	void Init(TrackInfo* TI, const bool& Fmt);
	void Clear();
};

class Extractor : public FXThread, FXObject
{
protected:
	Extractor();
	
	Extract_Vals V;

	ListEntry<TrackInfo>* CurTrack;
	short Cur, Last;

	FXString DumpFN;	// Temporary extraction wave file, created in a temporary directory (e.g. extract.wav)
	FXString EncFN; 	// Temporary path to the encoded file in the temporary directory (e.g. extract.mp3)
	FXString Ext;		// Final file extension
	
	bool Move(FXString& Dest, FXString& Src);	// Moves [Src] to [Dest] and returns true if successful. It's that simple.
	uint Tag(TrackInfo* TI, FXString& OutFN);	// Cross-format tagging
	bool Finish();	// Only called by the thread function

public:
	Encoder* Enc;

	static volatile FXuint Ret;
	
	// Helper functions
	// ------
	// Ensures [In] to have the (original) PCM data from [TI]. Decodes the file, if necessary.
	bool PrepareInput(TrackInfo* TI, GameInfo* GI, Extract_Vals& V);

	// Writes a looped and faded PCM dump of [TI], according to [V]. Returns it's filename
	FXString& BuildPCM(TrackInfo* TI, Extract_Vals& V);
	// ------

	bool ExtractTrack(TrackInfo* TI, FXString& OutFN);	// Single track extraction main function

	bool Active;
	List<FadeAlg*>	FAs;

	virtual FXint run();	// FOX Thread function. Runs the extraction loop

	bool Start(const short& ExtStart, const short& ExtEnd, const ushort& FadeAlgID);
	void Stop();

	uint Cleanup();	// Removes temporary files

	SINGLETON(Extractor);

};

// Decryption thread.
// Used to speed up the extraction process, because we don't need seeking there.
// The caller links the thread to a progress variable.
// This way, decrypted bytes can be used immediately.
// -----------------

class VFile;

class Decrypter : public FXThread, FXObject
{
protected:
	Decrypter()	{}

	// Parameters
	FXString Src;
	GameInfo* GI;
	TrackInfo* TI;
	VFile* VF;

	virtual FXint run();

public:
	ulong Start(GameInfo* GI, TrackInfo* TI, VFile* VF);	// Starts threaded decryption, returns size of decrypted file
	void Stop();

	SINGLETON(Decrypter);
};
// -----------------
