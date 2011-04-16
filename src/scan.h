// Music Room Interface
// --------------------
// scan.h - Track scanning and verification
// --------------------
// "©" Nmlgc, 2010-2011

#ifndef MUSICROOM_SCAN_H
#define MUSICROOM_SCAN_H

struct OggVorbis_File;

// Base class
// ----------
class TrackScanner
{
protected:
	TrackScanner()	{}

	static OggVorbis_File	SF;
	static FXFile			F;
	static FXuint			OpenFNHash;	// Comparison hash of [F] and [SF]
	
	virtual bool	Open(GameInfo* GI, TrackInfo* TI);	// Makes sure [F] or [SF] contains the file of [TI]
	virtual ListEntry<TrackInfo>*	Init(GameInfo* GI);	// Initializes the scanner and returns the entry of the first track 

public:
	static bool	Close();
};
// ----------

// Seek test
// Verifies the number of BGM tracks
// ---------
class SeekTest : public TrackScanner
{
protected:
	SeekTest()	{}

	bool Track(GameInfo* GI, TrackInfo* TI);	// return whether a single track is present
	
public:
	bool Scan(GameInfo* GI);

	SINGLETON(SeekTest);
};
// ---------

// Silence scanner
// Finds the amount of leading silence on each track
// ---------------
class SilenceScan : public TrackScanner
{
protected:
	SilenceScan()	{}

	ulong Track_PCM(GameInfo *GI, TrackInfo *TI, ulong* Buf, ulong BufSize);
	ulong Track_Vorbis(GameInfo *GI, TrackInfo *TI, ulong* Buf, ulong BufSize);

	// Returns the amount of silence samples at the start of [TI]
	ulong Track(GameInfo* GI, TrackInfo* TI, ulong* Buf, ulong BufSize);

public:
	bool Scan(GameInfo* GI);
	
	SINGLETON(SilenceScan);
};

bool PerformScans(GameInfo* GI);

#endif /* MUSICROOM_SCAN_H */