// Music Room Interface
// --------------------
// tagger.h - Music Room specific tagging
// --------------------
// "©" Nmlgc, 2010-2011

// Forward declarations
class MRTag;

class Tagger : public FXThread, FXObject
{
protected:
	// Temporary string storage
	FXString Comment[LANG_COUNT];
	FXString Game[LANG_COUNT];
	FXString TN[2];
	FXString Genre;
	FXString Year;

	volatile bool StopReq;

	bool Search(TrackInfo* TI, const FXString& Ext, FXString* FN);	// Searches for a file which might match the given track

public:
	FXString Ext;
	bool Active;

	virtual FXint run();	// FOX Thread function. Runs the extraction loop

	MRTag*	NewFmtClass(const FXString& Ext);	// Gets a new tag class for the [Ext] extension

	bool TagBasic(MRTag* TF, GameInfo* GI, TrackInfo* TI);	// Writes only the basic tags 
	bool TagExt(MRTag* TF, GameInfo* GI, TrackInfo* TI);	// Writes comments and other language tags

	// Writes complete tags of [TI] for the [Ext] format to [TagFN]
	mrerr Tag(TrackInfo* TI, FXString& TagFN, FXString& Ext);
	void Stop();

	SINGLETON(Tagger);
};
