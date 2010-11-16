// Touhou Project BGM Extractor
// ----------------------------
// thbgmext.cpp - Main starting code
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"

#ifdef _WIN32 /* We need the following two to set stdin/stdout to binary */
#include <io.h>
#include <fcntl.h>
#endif

// Globals
// =======

ConfigFile MainCFG;

// GUI
// ---
MainWnd* MW;
FXApp* App;
ushort Lang;	// Current Tag Language (Japanese or English)
bool Play;	// Play selected track?
bool SilRem = true;	// Remove opening silence?
int Volume = 100;
// ---

// Update
// ------
bool WikiUpdate;
FXString WikiURL;
// ------

// Game
// ----
List<GameInfo> Game;	// Supported game array
GameInfo* ActiveGame = NULL;
// ----

// Encoders
// --------
List<Encoder> Encoders;
FXushort EncFmt;
bool ShowConsole; // Show encoding console during the process
// --------

PackMethod*	PM[PM_COUNT];

ushort LoopCnt;	// Song loop count (2 = song gets repeated once)
float FadeDur;	// Fade duration
FXString GamePath;
FXString AppPath;
FXString OutPath;	// Output directory

// =======


// String Constants
const FXString PrgName = "Touhou Project BGM Extractor";
const FXString NoGame = "(no game loaded)";
      FXString CfgFile = "thbgmext.cfg";
const FXString Example = "Example: ";
const FXString DumpFile = "extract";
const FXString DecodeFile = DumpFile + ".raw";
      FXString OGGDumpFile = "decode.ogg";
	  FXString OGGPlayFile = "play.ogg";
const FXString Trial[2] = {L" 体験版", " (Trial)"};
const FXString Cmp[2] = {L"作曲者", "Composer"};
const FXString WriteError = "ERROR: Couldn't get write access to file";

int main(int argc, char* argv[])
{
	// Setup pack methods
	PM[BGMDIR]	= &PM_BGMDir::Inst();
	PM[BGMDAT]	= &PM_BGMDat::Inst();
	PM[BMWAV]	= &PM_BMWav::Inst();
	PM[BMOGG]	= &PM_BMOgg::Inst();

	// This will fix the issue that our current directory stays at %HOMEPATH%
	// if somebody specifies a command line parameter via Explorer
	FXString FN, Dir = argv[0];

	FN = FX::FXPath::name(Dir);
	Dir.length(Dir.length() - FN.length());

	// Setup directories
	if(!Dir.empty())
	{
		FXSystem::setCurrentDirectory(Dir);
		AppPath = Dir;
	}
	else	AppPath = FXSystem::getCurrentDirectory() + PATHSEP;
	OGGDumpFile.prepend(FXSystem::getTempDirectory() + SlashString);
	OGGPlayFile.prepend(FXSystem::getTempDirectory() + SlashString);
	CfgFile.prepend(AppPath);

#ifdef _WIN32
	// We need to set stdin/stdout to binary mode. Damn windows.
	// Beware the evil ifdef. We avoid these where we can, but this one we
	// cannot. Don't add any more, you'll probably go to hell if you do.
	_setmode( _fileno( stdin ), _O_BINARY );
	_setmode( _fileno( stdout ), _O_BINARY );
#endif

	ReadConfig(CfgFile);

	FXApp _App(PrgName, "Nameless");
	_App.init(argc, argv);

	App = &_App;

	MW = new MainWnd(&_App);

	_App.create();

	if(argc > 1)	MW->LoadGame(argv[1]);
	
	MW->show(PLACEMENT_SCREEN);

	FXint Ret = _App.run();

	MainCFG.Save();
	MainCFG.Clear();

	Game.Clear();
	Encoders.Clear();

#ifdef WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
#endif

	return Ret;
}