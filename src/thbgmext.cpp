// Touhou Project BGM Extractor
// ----------------------------
// thbgmext.cpp - Main starting code
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"

#ifdef _WIN32 /* We need the following two to set stdin/stdout to binary */
#include <io.h>
#include <fcntl.h>
#endif

// Globals
// =======

// GUI
// ---
MainWnd* MW;
FXApp* App;
bool Lang;	// Current Tag Language (Japanese or English)
bool Play;	// Play selected track?
bool SilRem = true;	// Remove opening silence?
int Volume = 100;
// ---

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

PackMethod*	PM[BM_COUNT];

ushort LoopCnt;	// Song loop count (2 = song gets repeated once)
short FadeDur;	// Fade duration
FXString GamePath;
FXString AppPath;
FXString OutPath;	// Output directory

// =======

// String Constants
const FXString PrgName = "Touhou Project BGM Extractor";
const FXString NoGame = "(no game loaded)";
const FXString CfgFile = "thbgmext.cfg";
const FXString Example = "Example: ";
const FXString DumpFile = "extract";
const FXString DecodeFile = DumpFile + ".raw";
      FXString OGGDumpFile = "decode.ogg";

int main(int argc, char* argv[])
{
	// Setup pack methods
	PM[BGMDIR]	= &PM_BGMDir::Inst();
	PM[BGMDAT]	= &PM_BGMDat::Inst();
	PM[BMWAV]	= &PM_BMWav::Inst();
	PM[BMOGG]	= &PM_BMOgg::Inst();

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

	OGGDumpFile.prepend(FXSystem::getTempDirectory() + SlashString);

	MW = new MainWnd(&_App);

	_App.create();

	AppPath = FXSystem::getCurrentDirectory() + PATHSEP;

	MW->show(PLACEMENT_SCREEN);

	return _App.run();
}