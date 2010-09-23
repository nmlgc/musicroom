// Touhou Project BGM Extractor
// ----------------------------
// extract.h - Extracting and encoding functions
// ----------------------------
// "©" Nameless, 2010

extern const ushort WAV_HEADER_SIZE;
extern const ulong WAIT_INTERVAL;

class Extractor
{
protected:
	Extractor()	{}

	bool Active;
	short Cur, Last;

	ListEntry<TrackInfo>* CurTrack;

	FXString DumpFN;	// Temporary extraction wave file, created in a temporary directory (e.g. extract.wav)
	FXString EncFN; 	// Temporary path to the encoded file in the temporary directory (e.g. extract.mp3)
	FXString OutFN;		// Final encoded output file
	FXString Ext;		// Final file extension
	Encoder* Enc;

	uint Ret;	// Dialog box return value

#ifdef WIN32
	PROCESS_INFORMATION PI;
#endif

	uint ExtractStep1(TrackInfo* TI);	// Single track extraction main function
	void ExtractStep2(TrackInfo* TI);	// Stuff done after encoding
	uint Cleanup();	// Removes temporary files

public:
	bool ExtProc();	// Process callback

	bool ExtractTrack(TrackInfo* TI);	// Return value wrapper

	void Tag(TrackInfo* TI, FXString& TagFN, FXString& Ext);

	bool Start(short ExtStart, short ExtEnd);
	bool Next();	// Initiates extraction of the next track
	bool Finish();

	static Extractor& Inst()
	{
		static Extractor Instance;
		return Instance;
	}
};
