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

// Platform dependant macros
#ifndef WIN32

// Unix File System
#define ZeroMemory(p, s)    memset(p, 0, s)
#define DirSlash '/'
#define SlashString "/"
#define OtherDirSlash '\\'
#define OtherSlashString "\\"
const char LineEnd[2] = {'\n', '\0'};
const char OtherLineEnd[3] = {'\r', '\n', '\0'};
#else

// Windows File System
#define DirSlash '\\'
#define SlashString "\\"
#define OtherDirSlash '/'
#define OtherSlashString "/"
const char LineEnd[3] = {'\r', '\n', '\0'};
const char OtherLineEnd[2] = {'\n', '\0'};
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#define LANG_JP	0
#define LANG_EN 1

#include <fx.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "list.h"

struct TrackInfo
{
	ushort		Number; // Track Number (starting with 1!)

	FXString	FN;	// Individual Track Filename (only used with BGMDIR)
	FXString	Name[2]; // Track name in both languages
	FXString	Comment[2]; // Music room comment in both languages
	FXString	Artist;	// Composer of this track  - if the whole soundtrack was composed by one artist, the element of GameInfo is used instead

	ulong	FS;		// File size for compressed (read: Vorbis) tracks
	bool	Vorbis;	// Yes, subclassing would be nicer, but I'm tired now

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
	
	TrackInfo()	{Clear();}
};

struct GameInfo
{
	bool	Scanned;

	FXString	Name[2];	// Game name in both languages
	ushort	Year;
	ushort	PackMethod;
	FXString	GameNum;	// Game Number (e.g. "12.5")
	FXString	BGMFile;	// (not used with BGMDIR)
	FXString	BGMDir;	// BGM Subdirectory (only used with BGMDIR)
	ushort	HeaderSize;	// Header size of each BGM file (only used with BGMDIR)
	FXString	Artist;	// Composer of the whole soundtrack - if there are multiple composers for each track, the element of TrackInfo is used instead
	List<TrackInfo>	Track;

	uchar	ZWAVID[2];	// 0x8 and 0x9 in thbgm.dat for this game (only used with BGMDAT)

	FXString	GetTrackFN(TrackInfo* TI);
};

struct Encoder
{
	bool		Lossless;	// Lossless encoder?
	FXString	Name;	// Name displayed in the dialog
	FXString	CmdLine[2];	// Encoding command line
};

#include "parse.h"
#include "extract.h"
#include "mainwnd.h"

// Globals
// =======

// GUI
// ---
extern MainWnd* MW;
extern bool Lang;	// Current Tag Language (Japanese or English)
extern bool Play;	// Play selected track?
extern bool SilRem;	// Remove opening silence?
extern int Volume;
// ---

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

extern PackMethod*	PM[BM_COUNT];

extern ushort LoopCnt;	// Song loop count (2 = song gets repeated once)
extern short FadeDur;	// Fade duration
extern FXString GamePath;
extern FXString AppPath;
extern FXString OutPath;	// Output directory

// =======

// String Constants
extern const FXString PrgName;
extern const FXString NoGame;
extern const FXString Example;
extern const FXString DumpFile;
extern const FXString DecodeFile;
extern       FXString OGGDumpFile;

// Functions which are declared somewhere
ulong SplitString(const char* String, const char Delimiter, PList<char>* Result);
FXint BaseCheck(FXString& Str);
int ReadLineFromFile(char* String, int MaxChars, FILE* File);