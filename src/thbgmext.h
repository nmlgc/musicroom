// Touhou Project BGM Extractor
// ----------------------------
// "©" Nameless, 2010

// Definitions
// -----------
#define MIN(a, b)		        ((a) < (b) ? (a) : (b))						// Minimum
#define MAX(a, b)               ((a) > (b) ? (a) : (b))						// Maximum
#define BETWEEN(a, b, c)        ((a) > (b) && (a) < (c) ? true : false)		// Checks if a is between b and c
#define BETWEEN_EQUAL(a, b, c)	((a) >= (b) && (a) <= (c) ? true : false)	// Checks if a is between b and c or is equals b/c 
#define SAFE_DELETE(x)          {if(x) {delete   (x); (x) = NULL;}}			// Safe deletion
#define SAFE_DELETE_ARRAY(x)	{if(x) {delete[] (x); (x) = NULL;}}			// Safe deletion of an array

#define SINGLETON(Class)	protected: Class() {} public: static Class& Inst()	{static Class Inst;	return Inst;}

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// Platform dependant macros
#ifndef WIN32

// Unix File System
#define ZeroMemory(p, s)    memset(p, 0, s)
#define DirSlash '/'
#define SlashString "/"
#define OtherDirSlash '\\'
#define OtherSlashString "\\"
const char LineEnd[1] = {'\n'};
const char OtherLineEnd[2] = {'\r', '\n'};
#else

// Windows File System
#define DirSlash '\\'
#define SlashString "\\"
#define OtherDirSlash '/'
#define OtherSlashString "/"
const char LineEnd[2] = {'\r', '\n'};
const char OtherLineEnd[1] = {'\n'};
#endif

const char LineBreak[2] = {'\r', '\n'};
const uchar utf8bom[3] = {0xEF, 0xBB, 0xBF};

#define LANG_JP	0
#define LANG_EN 1

#ifdef _DEBUG
#include <iostream>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <fx.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "list.h"

struct IntString
{
	FXString s[2];
	
	IntString()	{}
	IntString(FXString& a, FXString& b)	{s[0] = a; s[1] = b;}
	FXString& operator [] (ushort l)	{return s[l];}
};

struct TrackInfo
{
	ushort		Number; // Track Number (starting with 1!)

	FXString	FN;	// Individual Track Filename (only used with BGMDIR)
	IntString	Name; // Track name in both languages
	IntString	Comment; // Music room comment in both languages
	IntString	Afterword;	// Combined afterword comments
	ushort	CmpID;	// Composer ID of this track  - if the whole soundtrack was composed by one artist, the element of GameInfo is used instead

	ulong	FS;		// File size for compressed (read: Vorbis) tracks
	bool	Vorbis;

	// Absolute Track Positions
	ulong	Start[2];	// (with or without silence)
	ulong	Loop;
	ulong	End;

	void Clear()
	{
		Start[0] = Start[1] = 0;
		Loop = 0;
		End = 0;
		Vorbis = false;
	}

	ulong GetByteLength();
	FXString GetComment(ushort Lang)	{return Comment[Lang] + Afterword[Lang];}
	
	TrackInfo()	{Clear();}
};

struct GameInfo
{
	bool	HaveTrackData;
	bool	Scanned;
	
	FXString	InfoFile;
	FXString	WikiPage;
	ulong		WikiRev;	// Wiki revision ID of the info file

	IntString	Name;	// Game name in both languages
	ushort	Year;
	ushort	PackMethod;
	FXString	GameNum;	// Game Number (e.g. "12.5")
	FXString	BGMFile;	// (not used with BGMDIR)
	FXString	BGMDir;	// BGM Subdirectory (only used with BGMDIR)
	ushort	HeaderSize;	// Header size of each BGM file (only used with BGMDIR)
	IntString	Artist;	// Composer of the whole soundtrack - if there are multiple composers for each track, the element of TrackInfo is used instead
	IntString	Circle;
	List<IntString> Composer;
	List<TrackInfo>	Track;
	ushort	TrackCount;	// Actual number of active tracks. Gets adjusted for trial versions.

	uchar	ZWAVID[2];	// 0x8 and 0x9 in thbgm.dat for this game (only used with BGMDAT)
	uchar	CryptKind;	// Encryption kind (only used with Tasofro games)
	ushort	EntrySize;	// Size of a junk-filled entry (only used with encryption version 1)

	bool ParseGameData(FXString InfoFile);	// Gets necessary data to identify the game
	bool ParseTrackData();	// Gets all the rest

	FXString	FullName(ushort Lang);
	FXString	GetTrackFN(TrackInfo* TI);

	void Clear();

	GameInfo();
	~GameInfo();
};

struct Encoder
{
	bool		Lossless;	// Lossless encoder?
	FXString	Name;	// Name displayed in the dialog
	FXString	CmdLine[2];	// Encoding command line
};

#include "parse.h"
#include "mainwnd.h"

// Globals
// =======

extern ConfigFile MainCFG;

// GUI
// ---
extern FXApp* App;
extern MainWnd* MW;
extern ushort Lang;	// Current Tag Language (Japanese or English)
extern bool Play;	// Play selected track?
extern bool SilRem;	// Remove opening silence?
extern int Volume;
// ---

// Update
// ------
extern bool WikiUpdate;
extern FXString WikiURL;
// ------

// Game
// ----
extern List<GameInfo> Game;	// Supported game array
extern GameInfo* ActiveGame;
// ----

// Encoders
// --------
extern List<Encoder> Encoders;
extern FXushort EncFmt;
extern bool ShowConsole; // Show encoding console during the process
// --------

extern PackMethod*	PM[PM_COUNT];

extern ushort LoopCnt;	// Song loop count (2 = song gets repeated once)
extern float FadeDur;	// Fade duration
extern FXString GamePath;
extern FXString AppPath;
extern FXString OutPath;	// Output directory

// =======

// String Constants
extern const FXString PrgName;
extern const FXString NoGame;
extern       FXString CfgFile;
extern const FXString Example;
extern const FXString DumpFile;
extern const FXString DecodeFile;
extern       FXString OGGDumpFile;
extern       FXString OGGPlayFile;
extern const FXString Trial[2];
extern const FXString Cmp[2];
extern const FXString WriteError;

// Functions which are declared somewhere
ulong SplitString(const char* String, const char Delimiter, PList<char>* Result);
FXint BaseCheck(FXString* Str);
int ReadLineFromFile(char* String, int MaxChars, FILE* File);
FXString& remove_sub(FXString& str, const FXchar* org, FXint olen);
FXString& remove_sub(FXString& str, const FXchar& org);
FXString& remove_sub(FXString& str, const FXString& org);
