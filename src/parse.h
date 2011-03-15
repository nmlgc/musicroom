// Music Room Interface
// --------------------
// parse.h - Main config file parsing and filename patterns
// --------------------
// "©" Nmlgc, 2010-2011

// Performs Wiki update for [GI]. Will ask for updates (BGMLib::UI_Update) if new data is present
bool Update(GameInfo* GI, FXString& WikiURL);

// Filename Patterns
// -----------------
extern FXString FNPattern;
extern const FXString Token[];
extern const FXchar TokenDelim;
// -----------------

class ConfigParser;

extern ConfigParser* LGD;	// Local Game Directory section in [LGDFile]