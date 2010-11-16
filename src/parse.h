// Touhou Project BGM Extractor
// ----------------------------
// parse.h - Info File Parsing, Directory Scans and Filename Patterns
// ----------------------------
// "©" Nameless, 2010

bool ParseDir(FXString Path);

GameInfo* ScanGame(FXString& Path);
bool Update(GameInfo* GI);	// return value => save updated BGM file

bool ReadConfig(const FXString& FN);

// Filename Patterns
// -----------------
extern FXString FNPattern;
extern const FXString Token[];
extern const FXchar TokenDelim;

FXString GetToken(ushort ID);
FXString PatternFN(TrackInfo* Track);
// -----------------

// Pack Methods
#define BGMDIR  0x0	// BGM Directory containing multiple wave files (th06, Kioh Gyoku)
#define BGMDAT  0x1	// raw PCM data in a single data file (usually thbgm.dat)
#define BMWAV   0x2	// wave files in a data file with header (only th075)
#define BMOGG   0x3	// Vorbis files in a data file with header (other Tasofro games)
#define PM_COUNT 0x4

// Encryptions
#define CR_NONE   0x0 // No encryption
#define CR_SUIKA  0x1 // Simple progressive XOR-encrypted header with lots of junk data, unencrypted files (th075, MegaMari)
#define CR_TENSHI 0x2 // Mersenne Twister pseudorandom encrypted header with an optional progressive XOR layer, static XOR-encrypted files (th105, th123, PatchCon)
#define CR_COUNT 0x2
// Yeah, that was the best I came up with :-)
// -----------

class ConfigFile;

// Pack Method Base Class
// ----------------------
class PackMethod
{
protected:
	List<GameInfo*>	PMGame;

public:
	virtual bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI) = 0;
	virtual bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack) = 0;		// return true if position data should be read

	virtual GameInfo* Scan(FXString& Path) = 0;	// Scans [Path] for a game packed with this method
	virtual void TrackData(GameInfo* GI) {}	// Another custom function after the tracks were parsed
};
// ----------------------

// PM_BGMDir
// ---------
class PM_BGMDir : public PackMethod
{
public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read

	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BGMDir);
};
// ---------

// PM_BGMDat
// ---------
class PM_BGMDat : public PackMethod
{
public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read

	bool CheckZWAVID(GameInfo* Target);

	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BGMDat);
};
// ---------

class PM_Tasofro : public PackMethod
{
private:
	ulong HeaderSizeV2(FX::FXFile& In);

	bool IsValidHeader(char* hdr, const FXuint& hdrSize, const FXushort& Files);

	void DecryptHeaderV1(char* hdr, const FXuint& hdrSize);
	void DecryptHeaderV2(char* hdr, const FXuint& hdrSize, const FXushort& Files);

protected:
	virtual void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize) = 0;

	ulong HeaderSize(GameInfo* GI, FX::FXFile& In, const FXushort& Files);
	bool DecryptHeader(GameInfo* GI, char* hdr, const FXuint& hdrSize, const FXushort& Files);

	bool CheckCryptKind(ConfigFile& NewGame, const uchar& CRKind);	// Checks if given encryption kind is valid

public:
	void TrackData(GameInfo* GI);
};

// PM_BMWav
// --------
class PM_BMWav : public PM_Tasofro
{
protected:
	bool CheckBMTracks(GameInfo* Target);
	void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize);

public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read
	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BMWav);
};
// --------

// PM_BMOgg
// --------

struct OggVorbis_File;

class PM_BMOgg : public PM_Tasofro
{
protected:
	void ReadSFL(FX::FXFile& In, ulong& Pos, ulong& Size, TrackInfo* TI);
	void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize);

public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read

	inline void DecryptBuffer(char* Out, ulong Pos, ulong Size);	// Contains the decryption algorithm

	char* DecryptFile(FX::FXFile& In, ulong& Pos, ulong& Size);
	bool DumpOGG(FX::FXFile& In, ulong& Pos, ulong& Size, FXString& DumpFN);

	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BMOgg);
};
// --------
