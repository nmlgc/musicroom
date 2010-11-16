// Touhou Project BGM Extractor
// ----------------------------
// extract.h - Extracting and encoding functions
// ----------------------------
// "©" Nameless, 2010

extern const ushort WAV_HEADER_SIZE;
extern const ulong WAIT_INTERVAL;

class Extractor : public FXThread, FXObject
{
private:
	// We don't want to leak everything in case we're canceled
	char* Buf;
	FX::FXFile In, Out;

protected:
	short Cur, Last;

	ListEntry<TrackInfo>* CurTrack;

	FXString DumpFN;	// Temporary extraction wave file, created in a temporary directory (e.g. extract.wav)
	FXString EncFN; 	// Temporary path to the encoded file in the temporary directory (e.g. extract.mp3)
	FXString Ext;		// Final file extension
	
#ifdef WIN32
	PROCESS_INFORMATION PI;
#endif

	uint ExtractStep1(TrackInfo* TI, FXString& OutFN);	// Single track extraction main function
	uint ExtractStep2(TrackInfo* TI, FXString& OutFN);	// Stuff done after encoding
	bool Finish();	// Only called by the thread function

public:
	Encoder* Enc;
	bool Active;
	volatile uint Ret;	// Dialog box return value

	virtual FXint run();	// FOX Thread function. Runs the extraction loop

	bool Start(short ExtStart, short ExtEnd);
	void Stop(FXString* Msg = NULL, bool ThreadCall = false);

	uint Cleanup();	// Removes temporary files

	SINGLETON(Extractor);

};

class Tagger : public FXThread, FXObject
{
protected:
	volatile bool StopReq;

	bool FLACTag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang);
	bool OGGTag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang);
	bool MP3Tag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang);

	bool Search(TrackInfo* TI, const FXString& Ext, FXString* FN);	// Searches for a file which might match the given track

public:
	FXString Ext;
	bool Active;

	virtual FXint run();	// FOX Thread function. Runs the extraction loop

	bool Tag(TrackInfo* TI, FXString& TagFN, FXString& Ext);
	void Stop();

	SINGLETON(Tagger);
};
