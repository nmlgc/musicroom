﻿// Music Room Interface
// --------------------
// musicroom.cpp - Main starting code
// --------------------
// "©" Nmlgc, 2010-2011

#include "musicroom.h"
#include <math.h>
#include <fx.h>
#include "mainwnd.h"
#include <bgmlib/config.h>
#include <bgmlib/bgmlib.h>

#ifdef PROFILING_LIBS
LARGE_INTEGER operator - (const LARGE_INTEGER& a, const LARGE_INTEGER& b)	{LARGE_INTEGER r; r.QuadPart = a.QuadPart - b.QuadPart; return r;}
LARGE_INTEGER operator + (const LARGE_INTEGER& a, const LARGE_INTEGER& b)	{LARGE_INTEGER r; r.QuadPart = a.QuadPart + b.QuadPart; return r;}
#endif

bool ReadConfig(ConfigFile* Cfg);
void SetupPM();

// Globals
// =======

ConfigFile MainCFG;
ConfigParser* LGD;	// Local Game Directory section in [LGDFile]

// GUI
// ---
MainWnd* MWBack;	// Back end GUI class
bool Play;	// Play selected track?
ushort FadeAlgID = 0;	// Fade algorithm
bool SilRem = true;	// Remove opening silence?
int Volume = 100;
// ---

// Game
// ----
GameInfo* ActiveGame = NULL;
// ----

// [update]
// --------
bool WikiUpdate;
FXString WikiURL;
// --------

// Encoders
// --------
List<Encoder*> Encoders;
FXushort EncFmt;
bool ShowConsole; // Show encoding console during the process
// --------

ushort LoopCnt;	// Song loop count (2 = song gets repeated once)
float FadeDur;	// Fade duration
FXString AppPath;
FXString OutPath;	// Output directory

// =======

// String Constants
      FXString PrgName = "Music Room Interface";
const FXString PrgVer = "v2.0";
const FXString NoGame = "(no game loaded)";
      FXString CfgFile = "musicroom.cfg";
	  FXString LGDFile = "gamedirs.cfg";
const FXString Example = "Example: ";
const FXString DumpFile = "extract";
const FXString DecodeFile = DumpFile + ".raw";
      FXString OggDumpFile = "decode.ogg";
	  FXString OggPlayFile = "play.ogg";
const FXString Cmp[LANG_COUNT] = {L"作曲者", "Composer"};

int main(int argc, char* argv[])
{
	SetupPM();

	// This will fix the issue that our current directory stays at %HOMEPATH%
	// if somebody specifies a command line parameter via Explorer
	FXString FN, Dir = argv[0];

	// Setup directories
	// -----------------
	FN = FX::FXPath::name(Dir);
	Dir.length(Dir.length() - FN.length());

	if(!Dir.empty())
	{
		FXSystem::setCurrentDirectory(Dir);
		AppPath = Dir;
	}
	else	AppPath = FXSystem::getCurrentDirectory() + PATHSEP;

	OggDumpFile.prepend(FXSystem::getTempDirectory() + SlashString);
	OggPlayFile.prepend(FXSystem::getTempDirectory() + SlashString);
	CfgFile.prepend(AppPath);
	LGDFile.prepend(AppPath);
	// -----------------

	// Read config files
	// -----------------
	ConfigFile MainCFG(CfgFile);
	ReadConfig(&MainCFG);

	ConfigFile LocalGameDir(LGDFile);
	LocalGameDir.Load();
	LGD = LocalGameDir.CheckSection("gamedirs");
	// -----------------

	FXApp App(PrgName, "Nmlgc");
	App.init(argc, argv);

	PrgName += " " + PrgVer;

	MWBack = new MainWnd(&App);
	MW = &MainWndFront::Inst();
	App.create();

	MWBack->show(PLACEMENT_SCREEN);

	if(argc > 1)	MWBack->handle(NULL, FXSEL(SEL_COMMAND, MainWnd::MW_LOAD_GAME), argv[1]);

	FXint Ret = App.run();

	LocalGameDir.Save();
	LocalGameDir.Clear();

	MainCFG.Save();
	MainCFG.Clear();

	// Clear encoders
	ListEntry<Encoder*>* CurEnc = Encoders.First();
	while(CurEnc)
	{
		SAFE_DELETE(CurEnc->Data);
		CurEnc = CurEnc->Next();
	}
	Encoders.Clear();

	MW->Clear();

	BGMLib::Clear();

#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif

	return Ret;
}