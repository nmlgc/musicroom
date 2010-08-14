// Touhou Project BGM Extractor
// ----------------------------
// parse.h - Info File Parsing, Directory Scans and Filename Patterns
// ----------------------------
// "©" Nameless, 2010

void ParseGame(FXString InfoFile, GameInfo* GI);
bool ParseDir(FXString Path);

GameInfo* ScanGame(FXString& Path);

void ReadConfig(const FXString& FN);

// Filename Patterns
// -----------------
extern FXString FNPattern;
extern const FXString Token[];
extern const FXchar TokenDelim;

FXString GetToken(ushort ID);
FXString PatternFN(TrackInfo* Track);
// -----------------

// Pack Methods
#define BGMDIR 0x0	// BGM Directory (only th06)
#define BGMDAT 0x1	// single data file (usually thbgm.dat)
#define BMWAV  0x2	// Brightmoon file containing encrypted wave files (only th075)
#define BMOGG  0x3	// Brightmoon file containing encrypted Vorbis files (other Tasofro games)
#define BM_COUNT 0x4
// -----------

class ConfigFile;

// Pack Method Base Class
// ----------------------
class PackMethod
{
protected:
	// Let's be comfortable
	static char TempStr[1024];

	List<GameInfo*>	PMGame;

public:
	virtual void ParseGameInfo(ConfigFile& NewGame, GameInfo* GI) = 0;
	virtual bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack) = 0;		// return true if position data should be read

	virtual GameInfo* Scan(FXString& Path) = 0;	// Scans [Path] for a game packed with this method
};
// ----------------------

// PM_BGMDir
// ---------
class PM_BGMDir : public PackMethod
{
public:
	void ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
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
	void ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read

	bool CheckZWAVID(GameInfo* Target);

	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BGMDat);
};
// ---------

class PM_Tasofro : public PackMethod
{
protected:
	virtual ulong GetHeaderSize(FX::FXFile& In, FXushort& Files) = 0;

	virtual void DecryptBMHeader(char* hdr, FXuint hdrSize) = 0;
	virtual void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize) = 0;

public:
	void GetHeader(GameInfo* GI);
};

// PM_BMWav
// --------
class PM_BMWav : public PM_Tasofro
{
protected:
	bool CheckBMTracks(GameInfo* Target);

	ulong GetHeaderSize(FX::FXFile& In, FXushort& Files);

	void DecryptBMHeader(char* hdr, FXuint hdrSize);
	void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize);

public:
	void ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
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
	ulong GetHeaderSize(FX::FXFile& In, FXushort& Files);

	void DecryptBMHeader(char* hdr, FXuint hdrSize);
	
	void ReadSFL(FX::FXFile& In, ulong& Pos, ulong& Size, TrackInfo* TI);
	void GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize);

public:
	void ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, const char* TN, TrackInfo* NewTrack);		// return true if position data should be read

	inline void DecryptBuffer(char* Out, ulong Pos, ulong Size);	// Contains the decryption algorithm

	char* DecryptFile(FX::FXFile& In, ulong& Pos, ulong& Size);
	bool DumpOGG(FX::FXFile& In, ulong& Pos, ulong& Size);

	GameInfo* Scan(FXString& Path);	// Scans [Path] for a game packed with this method

	SINGLETON(PM_BMOgg);
};
// --------
