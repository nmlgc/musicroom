// Touhou Project BGM Extractor
// ----------------------------
// parse.cpp - Info File Parsing
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"
#include "stream.h"

// GameInfo
// --------
void GameInfo::Clear()
{
	Composer.Clear();
	Track.Clear();
	HaveTrackData = false;
	Scanned = false;
}

GameInfo::GameInfo()
{
	HaveTrackData = false;
	Scanned = false;
}

FXString GameInfo::FullName(ushort Lang)
{
	if(TrackCount == Track.Size())	return Name[Lang];
	else							return Name[Lang] + Trial[Lang];
}

GameInfo::~GameInfo()
{
	Clear();
}
// --------

ulong ParseSubstring(char* Split)
{
	FXString Conv = Split;
	return Conv.toULong(BaseCheck(&Conv));
}

bool GameInfo::ParseGameData(FXString Info)
{
	FXString Str;

	InfoFile = Info;
	
	ConfigFile NewGame(InfoFile.text());
	NewGame.Load();

	// Game Info
	// ---------
	NewGame.GetValue("game", "name", TYPE_STRING, &Name[LANG_JP]);
	NewGame.GetValue("game", "name_en", TYPE_STRING, &Name[LANG_EN]);
	NewGame.GetValue("game", "packmethod", TYPE_USHORT, &PackMethod);
	NewGame.GetValue("game", "encryption", TYPE_UCHAR, &CryptKind);
	NewGame.GetValue("game", "entrysize", TYPE_USHORT, &EntrySize);
	NewGame.GetValue("game", "headersize", TYPE_USHORT, &HeaderSize);
	NewGame.GetValue("game", "gamenum", TYPE_STRING, &GameNum);

	NewGame.GetValue("game", "tracks", TYPE_USHORT, &TrackCount);
	
	if(PackMethod > PM_COUNT)
	{
		Str.format("ERROR: Invalid BGM packing method specified in %s!\n", InfoFile);
		MW->PrintStat(Str);
		return false;
	}
	else if(!PM[PackMethod]->ParseGameInfo(NewGame, this))
	{
		return false;
	}

	Str.format("%s (%s)\n", Name[Lang], InfoFile.text());
	MW->PrintStat(Str);
	return true;
}

bool GameInfo::ParseTrackData()
{
	FXString Str, Key;
	ulong TempVal;

	PList<char> Split;
	PListEntry<char>* CurSS;

	TrackInfo* NewTrack;
	ushort Tracks = 0;
	FXString TrackNo;
	char* TN;

	Str = AppPath + InfoFile;

	ConfigFile NewGame(Str.text());
	NewGame.Load();

	// Game Info
	// ---------
	NewGame.GetValue("game", "circle", TYPE_STRING, &Circle[LANG_JP]);
	NewGame.GetValue("game", "circle_en", TYPE_STRING, &Circle[LANG_EN]);
	NewGame.GetValue("game", "artist", TYPE_STRING, &Artist[LANG_JP]);
	NewGame.GetValue("game", "artist_en", TYPE_STRING, &Artist[LANG_EN]);
	NewGame.GetValue("game", "year", TYPE_USHORT, &Year);

	if(Artist[LANG_EN].empty())	Artist[LANG_EN] = Artist[LANG_JP];
	if(Circle[LANG_EN].empty())	Circle[LANG_EN] = Circle[LANG_JP];
	// ---------

	// Composers
	// ---------
	ListEntry<IntString>* New = NULL;
	ushort Cur = 1;

	Str.format("cmp%d", Cur);
	while(NewGame.GetValue("composer", Str.text(), TYPE_STRING, &Str))
	{
		New = Composer.Add();
		New->Data[LANG_JP] = Str;

		Str.format("cmp%d_en", Cur);
		if(!NewGame.GetValue("composer", Str.text(), TYPE_STRING, &New->Data[LANG_EN]))	New->Data[LANG_EN] = New->Data[LANG_JP];

		Str.format("cmp%d", ++Cur);
	}

	if(!New)
	{
		New = Composer.Add();
		New->Data[LANG_JP] = Artist[LANG_JP];
		New->Data[LANG_EN] = Artist[LANG_EN];
	}
	// ---------

	// Track Info
	// ---------

	NewGame.GetValue("game", "tracks", TYPE_USHORT, &Tracks);

	for(ushort c = 0; c < Tracks; c++)
	{
		NewTrack = &(Track.Add()->Data);
		NewTrack->Clear();
		NewTrack->Number = c + 1;

		TrackNo.fromInt(c + 1);
		if((c + 1) < 10)	TrackNo.prepend('0');

		TN = TrackNo.text();

		NewGame.LinkValue(TN, "name_jp", TYPE_STRING, &NewTrack->Name[LANG_JP]);
		NewGame.LinkValue(TN, "name_en", TYPE_STRING, &NewTrack->Name[LANG_EN]);

		if(!NewGame.GetValue(TN, "composer", TYPE_USHORT, &NewTrack->CmpID))	NewTrack->CmpID = 1;
		NewTrack->CmpID--;
				
		NewGame.LinkValue(TN, "comment_jp", TYPE_STRING, &NewTrack->Comment[LANG_JP]);
		NewGame.LinkValue(TN, "comment_en", TYPE_STRING, &NewTrack->Comment[LANG_EN]);

		ushort Cmt = 2, Rep = 7;
		Key.format("comment%d_jp", Cmt);
		while(NewGame.GetValue(TN, Key.text(), TYPE_STRING, &Str))
		{
			NewTrack->Afterword[LANG_JP].append("\n\n" + Str);

			Key.format("comment%d_en", Cmt);
			if(NewGame.GetValue(TN, Key.text(), TYPE_STRING, &Str))		NewTrack->Afterword[LANG_EN].append("\n\n" + Str);

			Key.format("comment%d_jp", ++Cmt);
		}

		if(!PM[PackMethod]->ParseTrackInfo(NewGame, this, TN, NewTrack))	continue;

		// Read position data
		// ------------------

		NewGame.GetValue(TN, "start", TYPE_ULONG, &NewTrack->Start[0]);

		// Absolute values
		NewGame.GetValue(TN, "abs_loop", TYPE_ULONG, &NewTrack->Loop);
		NewGame.GetValue(TN, "abs_end", TYPE_ULONG, &NewTrack->End);

		// Relative values
		NewGame.GetValue(TN, "rel_loop", TYPE_ULONG, &TempVal);	NewTrack->Loop = NewTrack->Start[0] + TempVal;
		NewGame.GetValue(TN, "rel_end", TYPE_ULONG, &TempVal);	NewTrack->End = NewTrack->Start[0] + TempVal;

		// Coolier array format
		if(NewGame.GetValue(TN, "position", TYPE_STRING, &Str))
		{
			Split.Clear();
			if(SplitString(Str.text(), ',', &Split) == 3)
			{
				CurSS = Split.First();

				NewTrack->Start[0] = ParseSubstring(CurSS->Data[0] == ' ' ? CurSS->Data + 1 : CurSS->Data);	CurSS = CurSS->Next();
				NewTrack->Loop = ParseSubstring(CurSS->Data[0] == ' ' ? CurSS->Data + 1 : CurSS->Data); 	CurSS = CurSS->Next();
				NewTrack->End = ParseSubstring(CurSS->Data[0] == ' ' ? CurSS->Data + 1 : CurSS->Data);  	CurSS = CurSS->Next();

				NewTrack->Loop += NewTrack->Start[0];
				NewTrack->End += NewTrack->Start[0];
			}
		}

		if(!NewTrack->Start || !NewTrack->Loop || !NewTrack->End)
		{
			Str.format("WARNING: Couldn't read track position data for Track #%s%d %s!", (c+1) < 10 ? "0" : "", c+1, NewTrack->Name[Lang]);
			MW->PrintStat(Str);
		}
	}
	// ---------

	if(WikiUpdate)
	{
		NewGame.GetValue("update", "wikipage", TYPE_STRING, &WikiPage);
		NewGame.LinkValue("update", "wikirev", TYPE_ULONG, &WikiRev);
		if(Update(this))
		{
			Str.format("Saving new track information in %s...\n", FXPath::name(NewGame.GetFN()));
			MW->PrintStat(Str);
			if(!NewGame.Save())
			{
				Str.format("%s!\n", WriteError);
				MW->PrintStat(Str);
			}
		}
	}
	NewGame.Clear();

	HaveTrackData = true;

	return true;
}

bool ParseDir(FXString Path)
{
	FXString* BGM;
	FXint	BGMCount;

	GameInfo* NewGame;

	BGMCount = FXDir::listFiles(BGM, Path, "*.bgm");

	if(BGMCount == 0)
	{
		MW->PrintStat("ERROR: No BGM info files found in " + Path + "!\nGet some and then restart this application.\n");
		return false;
	}

	Game.Clear();

	MW->PrintStat("---------------------------\nSupported:\n");

	for(FXint c = 0; c < BGMCount; c++)
	{
		NewGame = &(Game.Add()->Data);
		NewGame->Scanned = false;
		if(!NewGame->ParseGameData(BGM[c]))	Game.PopLast();
	}

	SAFE_DELETE_ARRAY(BGM);

	MW->PrintStat("---------------------------\n\n");

	return true;
}

bool SeekTest(GameInfo* GI)
{
	if(GI->PackMethod == BGMDIR)	return true;

	ushort Seek = 0, Found = 0;
	TrackInfo* TI;
	FX::FXFile In;
	char Read;
	bool Ret = true;
	FXString Str;

	MW->PrintStat("\nVerifying track count...");

	In.open(GI->BGMFile, FXIO::Reading);

	ListEntry<TrackInfo>* CurTI = GI->Track.First();
	while(CurTI)
	{
		TI = &CurTI->Data;

		if(TI->Start[0] != 0)
		{
			if(GI->PackMethod == BGMDAT)	In.position(TI->End - 1);
			else							In.position(TI->Start[0]);

			if(In.readBlock(&Read, 1) == 1)
			{
				Seek = TI->Number;
				Found++;
			}
			else	break;
		}

		CurTI = CurTI->Next();
	}
	In.close();

	Str.format("%d/%d\n", Found, GI->Track.Size());

	if(Seek == 0)
	{
		Str.append("BGM file (" + GI->BGMFile + ") is corrupted!\n");
		Ret = false;
	}
	else if(Seek != GI->Track.Size())
	{
		Str.append("This is most likely a trial version. Extraction is limited to the available tracks.\n");
	}
	GI->TrackCount = Seek;
	MW->PrintStat(Str);
	return Ret;
}

void SilenceScan(GameInfo* GI)
{
	if(GI->Scanned)	return;

	MW->PrintStat("Scanning track start values...");

	TrackInfo* TI;
	FX::FXFile In;
	ulong c = 0;

	ulong Comp = 0;

	bool MultiFile = GI->BGMFile.empty();

	if(!MultiFile)	In.open(GI->BGMFile, FXIO::Reading);

	ListEntry<TrackInfo>* CurTI = GI->Track.First();
	for(ushort Temp = 0; Temp < GI->TrackCount; Temp++)
	{
		TI = &CurTI->Data;

		if(MultiFile)	In.open(GI->GetTrackFN(TI), FXIO::Reading);

		In.position(TI->Start[0]);
		FXuint BufSize = TI->Loop - TI->Start[0];

		char* Buf = new char[BufSize];
		In.readBlock(Buf, BufSize);

		for(c = 0; c < BufSize; c += 4)
		{
			if(memcmp(&Buf[c], &Comp, 4))	break;
		}
		// Fix Tasofro games
		if(c == BufSize)	c = 0;

		SAFE_DELETE_ARRAY(Buf);
		
		TI->Start[1] = TI->Start[0] + c;
		
		if(MultiFile)	In.close();

		CurTI = CurTI->Next();
	}

	if(!MultiFile)	In.close();

	GI->Scanned = true;

	MW->PrintStat("done.\n");
}

GameInfo* ScanGame(FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString Message;

	if(!FXSystem::setCurrentDirectory(Path))	FXSystem::setCurrentDirectory(Path = FXPath::directory(Path));

	MW->PrintStat("Scanning " + Path + "...\n");
	MW->PrintStat("------------------------\n");

	for(char m = PM_COUNT - 1; m > -1; m--)
	{
		if(ActiveGame = NewGame = PM[m]->Scan(Path))	break;
	}

	if(NewGame != NULL)
	{
		Message.format("Identified %s\n", NewGame->Name[Lang]);
		MW->PrintStat(Message);
		if(!NewGame->HaveTrackData)	NewGame->ParseTrackData();

		PM[NewGame->PackMethod]->TrackData(NewGame);

		if(SeekTest(NewGame))
		{
			if(!NewGame->Scanned)	SilenceScan(NewGame);
		}
		else NewGame = NULL;
	}
	else
	{
		MW->PrintStat("No compatible game found!\n");
	}

	MW->PrintStat("------------------------\n\n");

	FXSystem::setCurrentDirectory(AppPath);

	return NewGame;
}
// -----------------------

bool ReadConfig(const FXString& FN)
{
	ushort Index = 0;
	FXString Sect, Temp;
	Encoder* New;

	bool Ret = MainCFG.Load(FN.text());
	
	MainCFG.LinkValue("default", "lang", TYPE_USHORT, &Lang);
	MainCFG.LinkValue("default", "play", TYPE_BOOL, &Play);
	MainCFG.LinkValue("default", "showconsole", TYPE_BOOL, &ShowConsole);
	MainCFG.LinkValue("default", "removesilence", TYPE_BOOL, &SilRem);
	MainCFG.LinkValue("default", "loop", TYPE_USHORT, &LoopCnt);
	MainCFG.LinkValue("default", "fade", TYPE_FLOAT, &FadeDur);
	MainCFG.LinkValue("default", "volume", TYPE_INT, &Volume);
	MainCFG.LinkValue("default", "enc", TYPE_USHORT, &EncFmt);
	MainCFG.LinkValue("default", "pattern", TYPE_STRING, &FNPattern);
	MainCFG.LinkValue("default", "outdir", TYPE_STRING, &OutPath);

	MainCFG.LinkValue("update", "wikiupdate", TYPE_BOOL, &WikiUpdate);
	MainCFG.LinkValue("update", "wikiurl", TYPE_STRING, &WikiURL);

	Sect.format("enc%d", ++Index);
	while(MainCFG.GetValue(Sect.text(), "name", TYPE_STRING, &Temp))
	{
		New = &(Encoders.Add()->Data);
		New->Name = Temp;

		MainCFG.GetValue(Sect.text(), "lossless", TYPE_BOOL, &New->Lossless);
		MainCFG.LinkValue(Sect.text(), "encoder", TYPE_STRING, &New->CmdLine[0]);
		MainCFG.LinkValue(Sect.text(), "options", TYPE_STRING, &New->CmdLine[1]);

		Sect.format("enc%d", ++Index);
	}
	return Ret;
}

// Filename Patterns
// -----------------
FXString FNPattern;
const FXString Token[] = {"number", "name_jp", "name_en", "name"};
const FXchar TokenDelim = '%';

FXString GetToken(ushort ID)
{
	FXString r;

	if(ID > ARRAYNUMBER(Token))	return "";

	r.format("%c%s%c", TokenDelim, Token[ID], TokenDelim);
	return r;
}

FXString PatternFN(TrackInfo* Track)
{
	if(!Track)	return "";

	FXString FN = FNPattern;

	FXString TrackNo;
	TrackNo.fromInt(Track->Number);
	if((Track->Number) < 10)	TrackNo.prepend('0');

	FN.substitute(GetToken(0), TrackNo);
	FN.substitute(GetToken(1), Track->Name[LANG_JP]);
	FN.substitute(GetToken(2), Track->Name[LANG_EN]);
	FN.substitute(GetToken(3), Track->Name[Lang]);

	// Remove illegal chars
	FN.substitute('/', '_');

	// People would be very angry if U.N. Owen doesn't appear
	remove_sub(FN, "?");
	
	// Append encoder
	if(EncFmt > 0)
	{
		FN.append(".");
		FN.append(Encoders.Get(EncFmt - 1)->Data.Name.lower());
	}

	return FN;
}
// -----------------
