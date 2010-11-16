// Touhou Project BGM Extractor
// ----------------------------
// mainwnd.cpp - GUI
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "stream.h"

#include "extract.h"

// Modified Table Widget
// ---------------------
LOTable::LOTable(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb)
	: FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb)
{
}

void LOTable::fitColumnsToContents(FX::FXint col, FX::FXint nc)
{
	register FXint c;

	FXHeader* Cap = getColumnHeader();
	FXHeaderItem* HI;
	FXint HISize;

	for(c=col; c<col+nc; c++)
	{
		HISize = 0;
		HI = Cap->getItem(c);
		if(HI)	HISize = HI->getWidth(Cap);

		setColumnWidth(c, MAX(getMinColumnWidth(c), HISize) + 5);
	}
}
// ---------------------

// Message Map
FXDEFMAP(MainWnd) MMMainWnd[] =
{
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_CHANGE_ACTION_STATE, MainWnd::onChangeActionState),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_STOP_SHOW, MainWnd::onStopShow),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_PARSEDIR, MainWnd::onParseDir),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_LOADGAME, MainWnd::onLoadGame),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_FILLTABLE_BASIC, MainWnd::onFillTableBasic),
	FXMAPFUNCS(SEL_COMMAND, MainWnd::MW_UPDATE_STRINGS, MainWnd::MW_UPDATE_STRINGS_END, MainWnd::onCmdStrings),
	FXMAPFUNCS(SEL_UPDATE, MainWnd::MW_UPDATE_STRINGS, MainWnd::MW_UPDATE_STRINGS_END, MainWnd::onUpdStrings),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_UPDATE_LENGTHS, MainWnd::onCmdLengths),
	FXMAPFUNC(SEL_CHANGED, MainWnd::MW_UPDATE_LENGTHS, MainWnd::onCmdLengths),
	FXMAPFUNC(SEL_CHANGED, MainWnd::MW_CHANGE_TRACK, MainWnd::onChangeTrack),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_TOGGLE_PLAY, MainWnd::onTogglePlay),
	FXMAPFUNC(SEL_TIMEOUT, MainWnd::MW_STREAM, MainWnd::onStream),
	FXMAPFUNC(SEL_CHANGED, MainWnd::MW_FN_PATTERN, MainWnd::onFNPattern),
	FXMAPFUNCS(SEL_COMMAND, MainWnd::MW_UPDATE_ENC, MainWnd::MW_UPDATE_ENC_END, MainWnd::onUpdEnc),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_OUTDIR, MainWnd::onSelectOutDir),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_EXTRACT_ALL, MainWnd::onExtract),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_EXTRACT_SEL, MainWnd::onExtract),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_STOP, MainWnd::onStop),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_TAG_UPDATE, MainWnd::onTagUpdate),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_EXT_MSG, MainWnd::onExtMsg),
	FXMAPFUNC(SEL_COMMAND, MainWnd::MW_EXT_FINISH, MainWnd::onExtFinish),
};

FXIMPLEMENT(MainWnd, FXMainWindow, MMMainWnd, ARRAYNUMBER(MMMainWnd));

MainWnd::MainWnd(FXApp* App)
	: FXMainWindow(App, PrgName, NULL, NULL, DECOR_TITLE | DECOR_MINIMIZE | DECOR_MAXIMIZE | DECOR_CLOSE | DECOR_BORDER | DECOR_STRETCHABLE)
{
	CurTrack = NULL;
	EncBtn = NULL;
	CFGFail = false;

	FXColor TmpClr;
	FXString TmpStr;

	Main = new FXVerticalFrame(this, LAYOUT_FILL);

	WikiDT.connect(WikiUpdate);

	// Language
	LangBox = new FXGroupBox(Main, "Tag Language", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL_X);
		LangFrame = new FXHorizontalFrame(LangBox, LAYOUT_FILL);
		LangBT[0] = new FXRadioButton(LangFrame, "日本語", this, MW_UPDATE_STRINGS + LANG_JP);
		LangBT[1] = new FXRadioButton(LangFrame, "English", this, MW_UPDATE_STRINGS + LANG_EN);
		GetWiki = new FXCheckButton(LangFrame, "Get most recent track info from Touhou Wiki", &WikiDT, FXDataTarget::ID_VALUE, CHECKBUTTON_NORMAL | LAYOUT_RIGHT);
		GetWiki->update();

	// Game
	GameBox = new FXGroupBox(Main, "Game", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL);
		GameLabel = new FXLabel(GameBox, NoGame, 0, LABEL_NORMAL | LAYOUT_FILL_X | LAYOUT_CENTER_X);

		GameDirFrame = new FXHorizontalFrame(GameBox, LAYOUT_FILL_X);
		GameDirLabel = new FXLabel(GameDirFrame, "", 0, LABEL_NORMAL | LAYOUT_FILL_X | LAYOUT_LEFT | JUSTIFY_LEFT | JUSTIFY_BOTTOM, 0, 0, 0, 0, DEFAULT_PAD, DEFAULT_PAD *4, DEFAULT_PAD, DEFAULT_PAD);
		GameDirSelect = new FXButton(GameDirFrame, "Select Game Directory...", NULL, this, MW_LOADGAME, BUTTON_NORMAL | LAYOUT_RIGHT);

	// Track Overview
	LabelFrame = new FXHorizontalFrame(GameBox, LAYOUT_FILL_X, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, 0, 0, 0, 0);
	new FXLabel(LabelFrame, "Track Overview", NULL, LABEL_NORMAL | LAYOUT_LEFT);

#ifdef WIN32
		VolDT.connect(Volume);
		TrackVol = new FXSlider(LabelFrame, &VolDT, FXDataTarget::ID_VALUE, SLIDER_NORMAL | SLIDER_HORIZONTAL | SLIDER_ARROW_DOWN | SLIDER_TICKS_BOTTOM | LAYOUT_RIGHT | LAYOUT_FIX_WIDTH, 0, 0, 100);
		TrackPlay = new FXCheckButton(LabelFrame, "Play selected track", this, MW_TOGGLE_PLAY, CHECKBUTTON_NORMAL | LAYOUT_RIGHT, 0, 0, 0, 0, 0, 25);

		TrackVol->setRange(0, 100);
		TrackVol->setTickDelta(25);
#endif
		TrackFrame = new FXSplitter(GameBox, SPLITTER_HORIZONTAL | SPLITTER_TRACKING | LAYOUT_FILL);
		  TrackEdge = new FXPacker(TrackFrame, FRAME_SUNKEN | LAYOUT_MIN_WIDTH, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0);
		CommentEdge = new FXPacker(TrackFrame, FRAME_SUNKEN | LAYOUT_MIN_WIDTH, 0, 0, 300, 0, 0, 0, 0, 0, 0, 0);
		TrackView = new LOTable(TrackEdge, this, MW_CHANGE_TRACK, TABLE_COL_SIZABLE | LAYOUT_FILL);

		TmpClr = TrackView->getTextColor();
		TrackView->setTextColor(TrackView->getBackColor());
		TrackView->setBackColor(TmpClr);
		TrackView->setCellColor(0, 0, TmpClr);	TrackView->setCellColor(0, 1, TmpClr);
		TrackView->setCellColor(1, 0, TmpClr);	TrackView->setCellColor(1, 1, TmpClr);

		TrackView->insertColumns(0, 3);
		TrackView->setColumnText(0, "#");
		TrackView->setColumnText(1, "Name");
		TrackView->setColumnText(2, "Length");

		TrackView->setRowHeaderWidth(0);
		TrackView->setVisibleRows(6);

	// Comment
	Comment = new FXText(CommentEdge, NULL, 0, TEXT_READONLY | TEXT_WORDWRAP | LAYOUT_FILL);

		SetTextViewColors(Comment);
	// --------------

	// Output Directory
	OutDirBox = new FXGroupBox(Main, "Output Directory", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL_X);
		OutDirFrame = new FXHorizontalFrame(OutDirBox, LAYOUT_FILL_X);
		OutDirField = new FXTextField(OutDirFrame, 32, NULL, 0, TEXTFIELD_NORMAL | LAYOUT_FILL_X | LAYOUT_LEFT);
		OutDirField->setText(OutPath);

		new FXButton(OutDirFrame, "...", NULL, this, MW_OUTDIR, BUTTON_NORMAL | LAYOUT_RIGHT | LAYOUT_FILL_X);

	OutFrame = new FXHorizontalFrame(Main, LAYOUT_FILL_X);

	// Filename Pattern
	PatternBox = new FXGroupBox(OutFrame, "Filename Pattern", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL);
		FNField = new FXTextField(PatternBox, 32, this, MW_FN_PATTERN, TEXTFIELD_NORMAL);

		FNExample = new FXLabel(PatternBox, "");

		TmpStr.format("Tokens:\n\n"
			     "%s - Two-digit track number\n"
				 "%s - Track name (Japanese)\n"
				 "%s - Track name (English)\n"
				 "%s - Track name (current language)", GetToken(0), GetToken(1), GetToken(2), GetToken(3));

		new FXLabel(PatternBox, TmpStr, NULL, JUSTIFY_LEFT);

		FNField->setText(FNPattern);

	// Encoding Format
	EncDT.connect(EncFmt);

	EncBox = new FXGroupBox(OutFrame, "Output Format", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL_Y);
		EncFrame = new FXVerticalFrame(EncBox, LAYOUT_FILL);

		ListEntry<Encoder>* CurEnc = Encoders.First();
		FXushort EncIndex = 1;
		EncBtn = new FXRadioButton*[Encoders.Size()];
		while(CurEnc)
		{
			EncBtn[EncIndex - 1] = new FXRadioButton(EncFrame, CurEnc->Data.Name, this, MW_UPDATE_ENC + EncIndex, RADIOBUTTON_NORMAL | LAYOUT_CENTER_X | LAYOUT_CENTER_Y);
			EncIndex++;
			CurEnc = CurEnc->Next();
		}
		
		if(EncFmt > 0)	EncBtn[EncFmt - 1]->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);

		// We have to misuse this label as the general minimum dialog size,
		// because FXSplitter apparently doesn't care about LAYOUT_MIN_WIDTH...
		// EncMsg = new FXLabel(EncBox, "Encoding options can be configured in the thbgmext.cfg file. "
		                            //  "You can also use own encoders.", 0, LABEL_NORMAL | LAYOUT_CENTER_X | LAYOUT_FILL_X | LAYOUT_FIX_WIDTH, 0, 0, 600);

	// Parameters
	OutRightFrame = new FXVerticalFrame(OutFrame, LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	TagUpdate = new FXButton(OutRightFrame, "Update tags", NULL, this, MW_TAG_UPDATE, BUTTON_NORMAL | LAYOUT_FILL);

	ParamBox = new FXGroupBox(OutRightFrame, "Parameters", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_FILL_X);
		ParamFrame = new FXMatrix(ParamBox, 2, LAYOUT_RIGHT | MATRIX_BY_COLUMNS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		
		LoopFrame = new FXHorizontalFrame(ParamFrame, LAYOUT_FILL_X);

			LoopField = new FXSpinner(LoopFrame, 4, this, MW_UPDATE_LENGTHS, SPIN_NORMAL | FRAME_SUNKEN | FRAME_THICK | LAYOUT_RIGHT);
			new FXLabel(LoopFrame, "Loop Count: ", NULL, LABEL_NORMAL | LAYOUT_RIGHT);

		new FXHorizontalFrame(ParamFrame);

		FadeFrame = new FXHorizontalFrame(ParamFrame, LAYOUT_FILL_X);
			new FXLabel(ParamFrame, "seconds", NULL, LABEL_NORMAL | LAYOUT_RIGHT | LAYOUT_CENTER_Y);
			FadeField = new FXRealSpinner(FadeFrame, 4, this, MW_UPDATE_LENGTHS, SPIN_NORMAL | FRAME_SUNKEN | FRAME_THICK | LAYOUT_RIGHT);
			new FXLabel(FadeFrame, "Fade Duration: ", NULL, LABEL_NORMAL | LAYOUT_RIGHT);
		
		LoopField->setRange(1, 5);
		FadeField->setRange(-60, 60);
		FadeField->setIncrement(0.5);

		LoopField->setValue(LoopCnt);
		FadeField->setValue(FadeDur);

		RemoveSilence = new FXCheckButton(ParamFrame, "Remove opening silence", this, MW_UPDATE_LENGTHS, CHECKBUTTON_NORMAL);

	StartFrame = new FXVerticalFrame(OutRightFrame, LAYOUT_FILL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		StartAll = new FXButton(StartFrame, "Extract all", NULL, this, MW_EXTRACT_ALL, BUTTON_NORMAL | LAYOUT_FILL);
		StartSel = new FXButton(StartFrame, "Extract selected", NULL, this, MW_EXTRACT_SEL, BUTTON_NORMAL | LAYOUT_FILL);
		Stop = new FXButton(StartFrame, "Stop", NULL, this, MW_STOP, BUTTON_NORMAL | LAYOUT_FILL);

		Stop->hide();

	// Status
	StatBox = new FXGroupBox(Main, "Status", GROUPBOX_NORMAL | FRAME_GROOVE | LAYOUT_BOTTOM | LAYOUT_FILL);
		StatFrame = new FXVerticalFrame(StatBox, FRAME_SUNKEN | LAYOUT_FILL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		Stat = new FXText(StatFrame, NULL, 0, TEXT_READONLY | LAYOUT_FILL);

		Stat->setVisibleRows(8);
		SetTextViewColors(Stat);

	handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), false);
}

void MainWnd::SetTextViewColors(FXText* Text)
{
	FXColor TmpClr = Text->getBackColor();
	Text->setBackColor(Text->getTextColor());
	Text->setTextColor(TmpClr);
	Text->setCursorColor(TmpClr);
	Text->setBarColor(TmpClr);
}

void MainWnd::create()
{
	FXMainWindow::create();
	show(PLACEMENT_SCREEN);

	Streamer::Inst().Init();

	RemoveSilence->handle(this, FXSEL(SEL_COMMAND, SilRem ? ID_CHECK : ID_UNCHECK), (void*)SilRem);
	TrackPlay->handle(this, FXSEL(SEL_COMMAND, Play ? ID_CHECK : ID_UNCHECK), (void*)Play);
	handle(this, FXSEL(SEL_COMMAND, MW_TOGGLE_PLAY), (void*)Play);

	TrackView->fitColumnsToContents(0, 3);

	handle(this, FXSEL(SEL_COMMAND, MW_PARSEDIR), NULL);
}

long MainWnd::onChangeActionState(FXObject* Sender, FXSelector Message, void* ptr)
{
	bool State = ptr != 0;
	if(State)	{StartAll->enable();  StartSel->enable();  TagUpdate->enable();}
	else		{StartAll->disable(); StartSel->disable(); TagUpdate->disable();}
	return 1;
}

long MainWnd::onStopShow(FXObject* Sender, FXSelector Message, void* ptr)
{
	bool State = ptr != 0;
	if(State)	{StartAll->hide(); StartSel->hide(); Stop->show(); Stop->enable();}
	else		{StartAll->show(); StartSel->show(); Stop->hide();}
	StartAll->recalc();	StartSel->recalc();	Stop->recalc();
	return 1;
}

long MainWnd::onParseDir(FXObject* Sender, FXSelector Message, void* ptr)
{
	if(!ParseDir(FXSystem::getCurrentDirectory()))
	{
		GameDirSelect->disable();
	}

	if(CFGFail)
	{
		FXString Temp;
		Temp.format("Couldn't load the configuration file (%s)!\nExtraction won't be possible. You can still play back the tracks, though.\n", CfgFile);
		MW->PrintStat(Temp);
	}
	return 1;
}

GameInfo* MainWnd::LoadGame(FXString NewGD)
{
	Streamer::Inst().Stop();
	Streamer::Inst().CloseFile();

	GamePath = FX::FXPath::simplify(NewGD);
	NewGD = GamePath;	// Create a copy, GamePath gets changed in case of PM_BGMDir
	ActiveGame = ScanGame(GamePath);

	PrevTrackID = -1;
	
	if(!ActiveGame)
	{
		GameLabel->setText(NoGame);
		Comment->setText("");
		GameDirLabel->setText("");
		handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), (void*)false);
		RemoveSilence->enable();
	}
	else
	{
		CurTrack = &(ActiveGame->Track.First()->Data);

		handle(this, FXSEL(SEL_COMMAND, MW_FILLTABLE_BASIC), NULL);
		handle(this, FXSEL(SEL_COMMAND, MW_UPDATE_LENGTHS), NULL);

		SetComment(CurTrack);
		GameDirLabel->setText(NewGD);
		handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), (void*)true);

		if(ActiveGame->PackMethod == BMOGG)	RemoveSilence->disable();
		else								RemoveSilence->enable();
	}

	handle(FNField, FXSEL(SEL_CHANGED, MW_FN_PATTERN), NULL);

	return ActiveGame;
}

long MainWnd::onLoadGame(FXObject* Sender, FXSelector Message, void* ptr)
{
	FXString NewGameDir = FXDirDialog::getOpenDirectory(this, "Select Game Directory...", FXPath::upLevel(GamePath));
	if(NewGameDir.empty())	return 1;

	LoadGame(NewGameDir);
	
	return 1;
}

long MainWnd::onFillTableBasic(FXObject* Sender, FXSelector Message, void* ptr)
{
	FXint Row = 0;
	TrackInfo* Track;

	TrackView->removeRows(0, TrackView->getNumRows());
	if(!ActiveGame)	return 1;

	TrackView->insertRows(0, ActiveGame->TrackCount);

	ListEntry<TrackInfo>* CurTrack = ActiveGame->Track.First();
	for(ushort Temp = 0; Temp < ActiveGame->TrackCount; Temp++)
	{
		Track = &(CurTrack->Data);

		if(Track->Start[0] == 0)
		{
			TrackView->removeRows(Row);
			CurTrack = CurTrack->Next();
			continue;
		}

		TrackView->setItemJustify(Row, 1, FXTableItem::LEFT|FXTableItem::CENTER_Y);
		TrackView->setItemText(Row, 0, FXString::value(Track->Number));

		TrackView->getItem(Row, 0)->setData(Track);

		Row++;

		CurTrack = CurTrack->Next();
	}

	handle(this, FXSEL(SEL_COMMAND, MW_UPDATE_STRINGS + Lang), NULL);

	return 1;
}

long MainWnd::onCmdStrings(FXObject* Sender, FXSelector Message, void* ptr)
{
	Lang = (FXSELID(Message) - MW_UPDATE_STRINGS) != 0;

	LangBox->update();
	LangFrame->update();
	LangBT[0]->update();
	LangBT[1]->update();

	if(!ActiveGame)	return 1;

	GameLabel->setText(ActiveGame->FullName(Lang));
	
	// Update table strings
	FXint Row = 0;
	TrackInfo* Track;

	ListEntry<TrackInfo>* CurTI = ActiveGame->Track.First();
	for(ushort Temp = 0; Temp < ActiveGame->TrackCount; Temp++)
	{
		Track = &(CurTI->Data);

		if(Track->Start[0] != 0)
		{
			TrackView->setItemText(Row, 1, Track->Name[Lang]);
			Row++;
		}
		CurTI = CurTI->Next();
	}

	SetComment(CurTrack);
	TrackView->fitColumnsToContents(0, 3);

	handle(FNField, FXSEL(SEL_CHANGED, MW_FN_PATTERN), NULL);

	return 1;
}

long MainWnd::onUpdStrings(FXObject* Sender, FXSelector Message, void* ptr)
{
	ushort Send = (FXSELID(Message) - MW_UPDATE_STRINGS);
	Sender->handle(this, Send == Lang ? FXSEL(SEL_COMMAND, FXWindow::ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK), NULL);
	return 1;
}

long MainWnd::onCmdLengths(FXObject* Sender, FXSelector Message, void* ptr)
{
	FXint Row = 0;
	FXfloat SecLen;
	FXfloat Rem;
	FXString Str;
	
	LoopCnt = LoopField->getValue();
	FadeDur = FadeField->getValue();
	SilRem = RemoveSilence->getCheck() == TRUE;

	if(!ActiveGame)	return 1;

	ListEntry<TrackInfo>* CurTI = ActiveGame->Track.First();
	for(ushort Temp = 0; Temp < ActiveGame->TrackCount; Temp++)
	{
		if(CurTI->Data.Start[0] == 0)
		{
			CurTI = CurTI->Next();
			continue;
		}
		SecLen = (float)(CurTI->Data.GetByteLength() / 4) / 44100.0f;

		Rem = fmodf(SecLen, NULL);
		if(Rem < 0.5f)	SecLen = floorf(SecLen);
		else			SecLen = ceilf(SecLen);

		Str.format("%d:%2d", (ulong)(SecLen / 60.0f), ((ulong)SecLen) % 60);
		Str.substitute(' ', '0');

		TrackView->setItemText(Row, 2, Str);

		Row++;
		CurTI = CurTI->Next();
	}

	return 1;
}

void MainWnd::SetComment(TrackInfo* TI)
{
	if(TI)
	{
		FXString Cmt = TI->GetComment(Lang);
		Comment->setText(Cmt);
		if(!Cmt.empty())	Comment->appendText("\n – ");
		else				Comment->appendText(Cmp[Lang] + ": ");
		Comment->appendText(ActiveGame->Composer.Get(TI->CmpID)->Data[Lang], true);
	}
	else	Comment->setText("", 0, true);
}

long MainWnd::onChangeTrack(FXObject* Sender, FXSelector Message, void* ptr)
{
	LOTable* Table = (LOTable*)(Sender);

	FXint TrackID = Table->getCurrentRow();

	CurTrack = (TrackInfo*)Table->getItem(TrackID, 0)->getData();

	SetComment(CurTrack);

	handle(FNField, FXSEL(SEL_CHANGED, MW_FN_PATTERN), NULL);
	return 1;
}

#define STREAM_TIMEOUT 5000000

long MainWnd::onTogglePlay(FXObject* Sender, FXSelector Message, void* ptr)
{
	Play = ptr != 0;

	if(Play)	getApp()->addTimeout(this, MW_STREAM, STREAM_TIMEOUT);
	else	
	{
		// Streamer::Inst().Stop();
		getApp()->removeTimeout(this, MW_STREAM);
	}
	return 1;
}

long MainWnd::onStream(FXObject* Sender, FXSelector Message, void* ptr)
{
	getApp()->addTimeout(this, MW_STREAM, STREAM_TIMEOUT);
	if(!ActiveGame || !CurTrack)	return 1;

	Streamer& Str = Streamer::Inst();

	FXint NowTrackID = TrackView->getCurrentRow();
	if(NowTrackID != PrevTrackID)
	{
		Str.SwitchTrack(CurTrack);
	}

	Str.StreamFrame();

	PrevTrackID = NowTrackID;

	return 1;
}

long MainWnd::onFNPattern(FXObject* Sender, FXSelector Message, void* ptr)
{
	if(!ActiveGame)	return 1;

	FXString NewExample = "";

	FXTextField* Field = (FXTextField*)(Sender);
	FNPattern = Field->getText();

	if(CurTrack != NULL)	NewExample = PatternFN(CurTrack);
	else					NewExample = PatternFN(&(ActiveGame->Track.First()->Data));

	if(!NewExample.empty())
	{
		NewExample.prepend(Example);
		FNExample->setText(NewExample);
	}
	return 1;
}

long MainWnd::onUpdEnc(FXObject* Sender, FXSelector Message, void* ptr)
{
	FXushort Send = (FXSELID(Message) - MW_UPDATE_ENC - 1);

	for(ulong Count = 0; Count < Encoders.Size(); Count++)
	{
		if(Send != Count)	EncBtn[Count]->handle(this, FXSEL(SEL_COMMAND, ID_UNCHECK), NULL);
		else
		{
			EncFmt = Send + 1;
			EncBtn[Count]->handle(this, FXSEL(SEL_COMMAND, ID_CHECK), NULL);
		}
	}

	handle(FNField, FXSEL(SEL_CHANGED, MW_FN_PATTERN), NULL);

	return 1;
}

long MainWnd::onSelectOutDir(FXObject* Sender, FXSelector Message, void* ptr)
{
	OutPath = FXDirDialog::getOpenDirectory(this, "Select Output Directory...", FXPath::upLevel(GamePath));
	if(OutPath.empty())	return 1;

	OutDirField->setText(OutPath);

	return 1;
}

bool MainWnd::CheckOutDir()
{
	bool Empty = OutDirField->getText().empty();
	if(Empty)	FXMessageBox::error(getApp(), MBOX_OK, PrgName.text(), "Please specify an output directory.");
	else		OutPath = FX::FXPath::simplify(OutDirField->getText() + PATHSEP);
	return !Empty;
}

long MainWnd::onExtract(FXObject* Sender, FXSelector Message, void* ptr)
{
	short ExtStart = -1, ExtEnd = -1;
	bool Sel = FXSELID(Message) == MW_EXTRACT_SEL;

	// Directory Testing
	// -----------------
	if(!CheckOutDir())	return 1;
	if(!FX::FXSystem::setCurrentDirectory(OutPath))
	{
		if(!FX::FXDir::createDirectories(OutPath))
		{
			FXMessageBox::error(getApp(), MBOX_OK, PrgName.text(), "Error creating directory %s!", OutPath.text());
			return 1;
		}
	}
	FX::FXSystem::setCurrentDirectory(AppPath);
	// -----------------

	handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), false);	GameDirSelect->disable();
	handle(this, FXSEL(SEL_COMMAND, MW_STOP_SHOW), (void*)true);

	if(Sel)
	{
		ExtStart = TrackView->getSelStartRow();
		ExtEnd = TrackView->getSelEndRow();

		if(ExtStart == -1)
		{
			ExtStart = 0;
			ExtEnd = 1;
		}
	}
	else
	{
		ExtStart = 0;
		ExtEnd = ActiveGame->TrackCount + 1;
	}

	Extractor::Inst().Start(ExtStart, ExtEnd);

	return 1;
}

long MainWnd::onTagUpdate(FXObject* Sender, FXSelector Message, void* ptr)
{
	if(!CheckOutDir())	return 1;

	FXuint Ret;

	if(!FXSystem::setCurrentDirectory(OutPath))
	{
		FXMessageBox::error(getApp(), MBOX_OK, PrgName.text(), "Error entering directory %s!", OutPath.text());
		return 1;
	}

	Ret = FXMessageBox::question(this, MBOX_YES_NO, PrgName.text(),
		"This will update already extracted files from this game in the output directory with the current tag information.\n"
		"Please verify that the given filename pattern and the output format matches the files to be updated!\n\n"
		"Proceed?");

	if(Ret == MBOX_CLICKED_NO)	return 1;

	handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), (void*)false);	GameDirSelect->disable();
	handle(this, FXSEL(SEL_COMMAND, MW_STOP_SHOW), (void*)true);

	Tagger::Inst().start();

	return 1;
}

long MainWnd::onStop(FXObject* Sender, FXSelector Message, void* ptr)
{
	Extractor& Ext = Extractor::Inst();
	Tagger& Tag = Tagger::Inst();

	     if(Ext.Active)	Ext.Stop();
	else if(Tag.Active)
	{
		Tag.Stop();
		((FXButton*)Sender)->disable();
	}
	return 1;
}

long MainWnd::onExtMsg(FXObject* Sender, FXSelector Message, void* ptr)
{
	FXString* Str = (FXString*)ptr;
	Extractor& Ext = Extractor::Inst();

	Ext.Ret = FXMessageBox::question(this, MBOX_YES_YESALL_NO_NOALL_CANCEL, PrgName.text(), Str->text());

	Ext.resume();
	return 1;
}

long MainWnd::onExtFinish(FXObject* Sender, FXSelector Message, void* ptr)
{
	handle(this, FXSEL(SEL_COMMAND, MW_CHANGE_ACTION_STATE), (void*)true);
	handle(this, FXSEL(SEL_COMMAND, MW_STOP_SHOW), false);
	GameDirSelect->enable();
	FXSystem::setCurrentDirectory(AppPath);

	return 1;
}

void MainWnd::PrintStat(FXString NewStat)
{
	Stat->appendText(NewStat, true);
	Stat->makePositionVisible(Stat->rowStart(Stat->getLength()));
	Stat->update();

	getApp()->repaint();
}

void MainWnd::destroy()
{
	SAFE_DELETE_ARRAY(EncBtn);
	if(!OutDirField->getText().empty())	OutPath = FX::FXPath::simplify(OutDirField->getText() + PATHSEP);

	Streamer::Inst().Exit();
}
