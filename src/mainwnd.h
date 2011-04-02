// Music Room Interface
// --------------------
// mainwnd.h - GUI
// --------------------
// "©" Nmlgc, 2010-2011

// Action states
#define MW_ACT_NONE		0x00
#define MW_ACT_EXTRACT	0x01
#define MW_ACT_TAG		0x02
#define MW_ACT_ALL		MW_ACT_EXTRACT | MW_ACT_TAG

// Forward declarations
class LCListBox;
class LCTable;
class LCText;

class MainWnd : public FXMainWindow
{
	FXDECLARE(MainWnd);

	friend class Extractor;
	friend class MainWndFront;

private:
	MainWnd()	{}

protected:
	volatile bool Lock;
	short PrevTrackID;
	TrackInfo* CurTrack;

	// Widgets
	FXLabel*	GameDirLabel;
	FXButton*	GameDirSelect;

	FXCheckButton* TrackPlay;
	FXSlider* TrackVol;
	FXDataTarget VolDT;

	LCTable* TrackView;
	
	LCText* Comment;

	FXRadioButton*	LangBT[LANG_COUNT];

	FXDataTarget WikiDT;
	FXCheckButton*	GetWiki;

	FXTextField* FNField;
	FXLabel*	FNExample;
		FXSpinner* LoopField;
		FXRealSpinner* FadeField;
		FXDataTarget AlgDT;
		FXListBox*	AlgBox;
	FXCheckButton* RemoveSilence;

	FXTextField* OutDirField;
	FXButton*	OutDirSelect;

	FXButton* StartAll;
	FXButton* StartSel;
	FXButton* TagUpdate;
	FXButton* Stop;
	
	FXDataTarget	EncDT;
	FXRadioButton**	EncBtn;

	LCText*  	Stat;
	FXProgressBar*	ProgBar;
		FXDataTarget ProgDT;
	FXStatusLine*	PlayStat;

	bool CheckOutDir();

	void SetComment(TrackInfo* TI);

	FXString DirDialog(const FXString& Title);	// Shows a directory selection dialog and returns the selected directory

public:
	MainWnd(FXApp* App, FXIcon* AppIcon = NULL);

	FXString StatCache;	// PrintStat collector
	FXString Notice;	// Personal appeal cache
	LCListBox*	GameList;

	bool CFGFail;

	enum
	{
		MW_INIT = FXMainWindow::ID_LAST,
		// GUI
		MW_CHANGE_ACTION_STATE,
		MW_STOP_SHOW,
		MW_SHOW_NOTICE,

		MW_FILLTABLE_BASIC,

		MW_UPDATE_STRINGS,
		MW_UPDATE_STRINGS_END = MW_UPDATE_STRINGS + 1,
		MW_UPDATE_LENGTHS,

		MW_FN_PATTERN,

		MW_PARSEDIR,
		MW_SELECT_DIR,

		MW_LOAD_GAME,
		MW_SWITCH_GAME,
		
		MW_CHANGE_TRACK,
		MW_PROG_REDRAW,
		
		MW_TOGGLE_PLAY,
		MW_PLAY_STAT,

		MW_UPDATE_ENC,
		MW_UPDATE_ENC_END = MW_UPDATE_ENC + MAX_ENCODERS,

		MW_ENC_SETTINGS,

		MW_OUTDIR,
		// Actions
		MW_EXTRACT_ALL,
		MW_EXTRACT_SEL,
		MW_TAG_UPDATE,
		MW_STOP,
		MW_THREAD_STAT,
		MW_THREAD_MSG,
		MW_ACT_FINISH,

		ID_LAST 
	};

	void create();	// Inits the streamer and reads BGM info files
	void destroy();

	MESSAGE_FUNCTION(onChangeActionState);	// Changes the state of the extraction and tag buttons. [ptr] takes any valid action state value (see above)
	MESSAGE_FUNCTION(onStopShow);
	MESSAGE_FUNCTION(onShowNotice);
	MESSAGE_FUNCTION(onFillTableBasic);
	MESSAGE_FUNCTION(onCmdStrings);	// Updates all string labels with the set language
	MESSAGE_FUNCTION(onUpdStrings);	// Updates all string labels with the set language
	MESSAGE_FUNCTION(onCmdLengths);	// Calculate track lengths based on loop count and fade duration
	MESSAGE_FUNCTION(onFNPattern);
	MESSAGE_FUNCTION(onParseDir);
	MESSAGE_FUNCTION(onSelectDir);
	MESSAGE_FUNCTION(onLoadGame);	// Loads the game in the [ptr] directory
	MESSAGE_FUNCTION(onSwitchGame);
	MESSAGE_FUNCTION(onChangeTrack);
	MESSAGE_FUNCTION(onProgRedraw);
	MESSAGE_FUNCTION(onTogglePlay);
	MESSAGE_FUNCTION(onPlayStat);	// Displays playing status tooltip
	MESSAGE_FUNCTION(onStream);
	MESSAGE_FUNCTION(onUpdEnc);	// Selects a new encoder
	MESSAGE_FUNCTION(onEncSettings);	// Shows encoding settings dialog
	MESSAGE_FUNCTION(onSelectOutDir);
	MESSAGE_FUNCTION(onExtractTrack);
	MESSAGE_FUNCTION(onExtract);
	MESSAGE_FUNCTION(onTagUpdate);
	MESSAGE_FUNCTION(onStop);
	MESSAGE_FUNCTION(onThreadStat);	// Prints out collected stat messages from other threads
	MESSAGE_FUNCTION(onThreadMsg);
	MESSAGE_FUNCTION(onActFinish);

	void LoadGame(FXString& Path);
	void LoadGame(GameInfo* NewGame);

	void PrintStat(const FXString& NewStat);
	void ProgConnect(volatile FXulong* Var = NULL, FXuint Max = 0);	// Connects any variable to the progress bar at the bottom. Call again with [Var = NULL] to unresolve.
};

extern MainWnd* MWBack;	// Back end GUI class
