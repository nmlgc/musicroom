// Touhou Project BGM Extractor
// ----------------------------
// mainwnd.h - GUI
// ----------------------------
// "©" Nameless, 2010

#define MESSAGE_FUNCTION(x) long x(FXObject*, FXSelector, void*)

// Modified Table Widget
// ---------------------
class LOTable : public FXTable
{
public:
	LOTable(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);

	// Fit column widths to contents
	void fitColumnsToContents(FXint col,FXint nc=1);
};
// ---------------------

class MainWnd : public FXMainWindow
{
	FXDECLARE(MainWnd);

	friend class Extractor;

private:
	MainWnd()	{}

protected:
	short PrevTrackID;
	TrackInfo* CurTrack;

	// Widgets
	FXVerticalFrame*	Main;

	FXGroupBox*	GameBox;
		FXLabel*	GameLabel;
		FXHorizontalFrame* GameDirFrame;
		FXLabel*	GameDirLabel;
		FXButton*	GameDirSelect;

	FXHorizontalFrame* LabelFrame;
		FXCheckButton* TrackPlay;
		FXSlider* TrackVol;
		FXDataTarget VolDT;

	FXSplitter* TrackFrame;
		FXPacker* TrackEdge;
		LOTable* TrackView;

		FXPacker* CommentEdge;
		FXText* Comment;

	FXGroupBox*	LangBox;
		FXHorizontalFrame* LangFrame;
		FXRadioButton*	LangBT[2];

		FXDataTarget WikiDT;
		FXCheckButton*	GetWiki;

	FXGroupBox* PatternBox;
		FXTextField* FNField;
		FXLabel*	FNExample;

	FXHorizontalFrame* OutFrame;
	FXVerticalFrame* OutRightFrame;

	FXGroupBox* ParamBox;
		FXMatrix* ParamFrame;
		FXHorizontalFrame* LoopFrame;
			FXSpinner* LoopField;
		FXHorizontalFrame* FadeFrame;
			FXRealSpinner* FadeField;
		FXCheckButton* RemoveSilence;

	FXGroupBox* OutDirBox;
		FXHorizontalFrame* OutDirFrame;
		FXTextField* OutDirField;
		FXButton*	OutDirSelect;

	FXVerticalFrame* StartFrame;
		FXButton* StartAll;
		FXButton* StartSel;
		FXButton* TagUpdate;
		FXButton* Stop;
		
	FXGroupBox* EncBox;
		FXVerticalFrame* EncFrame;
		FXDataTarget	EncDT;
		FXRadioButton**	EncBtn;
		FXLabel*	EncMsg;

	FXGroupBox*	StatBox;
		FXVerticalFrame*	StatFrame;
		FXText*  	Stat;

	bool CheckOutDir();

	void SetTextViewColors(FXText* Text);	// Sets this cool inverted color scheme

	void SetComment(TrackInfo* TI);

public:
	MainWnd(FXApp* App);

	bool CFGFail;

	enum
	{
		// GUI
		MW_CHANGE_ACTION_STATE = FXMainWindow::ID_LAST,
		MW_STOP_SHOW,

		MW_FILLTABLE_BASIC,

		MW_UPDATE_STRINGS,
		MW_UPDATE_STRINGS_END = MW_UPDATE_STRINGS + 1,
		MW_UPDATE_LENGTHS,

		MW_FN_PATTERN,

		MW_PARSEDIR,
		MW_LOADGAME,
		
		MW_CHANGE_TRACK,
		
		MW_TOGGLE_PLAY,
		MW_STREAM,

		MW_UPDATE_ENC,
		MW_UPDATE_ENC_END = MW_UPDATE_ENC + 16,

		MW_OUTDIR,
		// Actions
		MW_EXTRACT_ALL,
		MW_EXTRACT_SEL,
		MW_TAG_UPDATE,
		MW_STOP,
		MW_EXT_MSG,
		MW_EXT_FINISH,

		ID_LAST 
	};

	void create();
	void destroy();

	MESSAGE_FUNCTION(onChangeActionState);
	MESSAGE_FUNCTION(onStopShow);
	MESSAGE_FUNCTION(onFillTableBasic);
	MESSAGE_FUNCTION(onCmdStrings);	// Updates all string labels with the set language
	MESSAGE_FUNCTION(onUpdStrings);	// Updates all string labels with the set language
	MESSAGE_FUNCTION(onCmdLengths);	// Calculate track lengths based on loop count and fade duration
	MESSAGE_FUNCTION(onFNPattern);
	MESSAGE_FUNCTION(onParseDir);
	MESSAGE_FUNCTION(onLoadGame);
	MESSAGE_FUNCTION(onChangeTrack);
	MESSAGE_FUNCTION(onTogglePlay);
	MESSAGE_FUNCTION(onStream);
	MESSAGE_FUNCTION(onUpdEnc);	// Updates all string labels with the set language
	MESSAGE_FUNCTION(onSelectOutDir);
	MESSAGE_FUNCTION(onExtractTrack);
	MESSAGE_FUNCTION(onExtract);
	MESSAGE_FUNCTION(onTagUpdate);
	MESSAGE_FUNCTION(onStop);
	MESSAGE_FUNCTION(onExtMsg);
	MESSAGE_FUNCTION(onExtFinish);

	GameInfo* LoadGame(FXString NewGameDir);

	void PrintStat(FXString NewStat);
};
