// Music Room Interface
// --------------------
// mainwnd.h - GUI
// --------------------
// "©" Nmlgc, 2010-2011

#ifndef MUSICROOM_MAINWD_H
#define MUSICROOM_MAINWD_H

// Action states
#define MW_ACT_NONE		0x00
#define MW_ACT_EXTRACT	0x01
#define MW_ACT_TAG		0x02
#define MW_ACT_ALL		MW_ACT_EXTRACT | MW_ACT_TAG

// Forward declarations
class LCListBox;
class LCTable;
class LCText;
class LCDirFrame;

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
	LCDirFrame* GameDir;
	
	FXCheckButton* TrackPlay;
	FXSlider* TrackVol;
	FXDataTarget VolDT;

	LCTable* TrackView;
	
	LCText* Comment;

	FXDataTarget WikiDT;
	FXCheckButton*	GetWiki;

	FXTextField* FNField;
	FXLabel*	FNExample;
		FXSpinner* LoopField;
		FXRealSpinner* FadeField;
		FXDataTarget AlgDT;
		FXListBox*	AlgBox;
	FXCheckButton* RemoveSilence;

	LCDirFrame* OutDir;

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

public:
	MainWnd(FXApp* App, FXIcon* AppIcon = NULL);

	FXString StatCache;	// PrintStat collector
	FXString Notice;	// Personal appeal cache
	LCListBox*	GameList;

	bool CFGFail;

	// Messages
	enum
	{
		MW_INIT = FXMainWindow::ID_LAST,
		// GUI
		MW_CHANGE_ACTION_STATE,
		MW_STOP_SHOW,
		MW_SHOW_NOTICE,

		MW_FILLTABLE_BASIC,

		MW_UPDATE_STRINGS,
		MW_UPDATE_LENGTHS,

		MW_FN_PATTERN,

		MW_LOAD_BGM_INFO,
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
	FXbool close(FXbool notify = true);

	MSG_FUNC(onChangeActionState);	// Changes the state of the extraction and tag buttons. [ptr] takes any valid action state value (see above)
	MSG_FUNC(onStopShow);
	MSG_FUNC(onShowNotice);
	MSG_FUNC(onFillTableBasic);
	MSG_FUNC(onCmdStrings);	// Updates all string labels with the set language
	MSG_FUNC(onUpdStrings);	// Updates all string labels with the set language
	MSG_FUNC(onCmdLengths);	// Calculate track lengths based on loop count and fade duration
	MSG_FUNC(onFNPattern);
	MSG_FUNC(onLoadBGMInfo);
	MSG_FUNC(onSelectDir);
	MSG_FUNC(onLoadGame);	// Loads the game in the [ptr] directory
	MSG_FUNC(onSwitchGame);
	MSG_FUNC(onChangeTrack);
	MSG_FUNC(onProgRedraw);
	MSG_FUNC(onTogglePlay);
	MSG_FUNC(onPlayStat);	// Displays playing status tooltip
	MSG_FUNC(onStream);
	MSG_FUNC(onUpdEnc);	// Selects a new encoder
	MSG_FUNC(onEncSettings);	// Shows encoding settings dialog
	MSG_FUNC(onExtractTrack);
	MSG_FUNC(onExtract);
	MSG_FUNC(onTagUpdate);
	MSG_FUNC(onStop);
	MSG_FUNC(onThreadStat);	// Prints out collected stat messages from other threads
	MSG_FUNC(onThreadMsg);
	MSG_FUNC(onActFinish);

	void LoadGame(FXString& Path);
	void LoadGame(GameInfo* NewGame);

	void PrintStat(const FXString& NewStat);
	void ProgConnect(volatile FXulong* Var = NULL, FXuint Max = 0);	// Connects any variable to the progress bar at the bottom. Call again with [Var = NULL] to unresolve.
};

extern MainWnd* MWBack;	// Back end GUI class

#endif /* MUSICROOM_MAINWD_H */