// Touhou Project BGM Extractor
// ----------------------------
// parse.cpp - Info File Parsing & Directory Scans
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"
#include "stream.h"

char PackMethod::TempStr[1024];

ulong ParseSubstring(char* Split)
{
	FXString Conv = Split;
	return Conv.toULong(BaseCheck(Conv));
}

void ParseGame(FXString InfoFile, GameInfo* GI)
{
	FXString Stat;
	char TempStr[2048];
	ulong TempVal;

	PList<char> Split;
	PListEntry<char>* CurSS;

	ConfigFile NewGame(InfoFile.text());

	TrackInfo* NewTrack;
	ushort Tracks = 0;
	FXString TrackNo;
	char* TN;

	NewGame.Load();

	// Game Info
	// ---------
	NewGame.GetValue("game", "name_jp", TYPE_STRING, (void*)TempStr);	GI->Name[LANG_JP] = TempStr;	TempStr[0] = '\0';
	NewGame.GetValue("game", "name_en", TYPE_STRING, (void*)TempStr);	GI->Name[LANG_EN] = TempStr;	TempStr[0] = '\0';
	NewGame.GetValue("game", "artist", TYPE_STRING, (void*)TempStr);	GI->Artist = TempStr;	TempStr[0] = '\0';
	NewGame.GetValue("game", "packmethod", TYPE_USHORT, (void*)&GI->PackMethod);
	NewGame.GetValue("game", "headersize", TYPE_USHORT, (void*)&GI->HeaderSize);
	NewGame.GetValue("game", "year", TYPE_USHORT, (void*)&GI->Year);
	NewGame.GetValue("game", "gamenum", TYPE_STRING, (void*)TempStr);	GI->GameNum = TempStr;	TempStr[0] = '\0';
	NewGame.GetValue("game", "tracks", TYPE_USHORT, (void*)&Tracks);

	PM[GI->PackMethod]->ParseGameInfo(NewGame, GI);
	// ---------

	// Track Info
	// ---------
	for(ushort c = 0; c < Tracks; c++)
	{
		NewTrack = &(GI->Track.Add()->Data);
		NewTrack->Clear();
		NewTrack->Number = c + 1;

		TrackNo.fromInt(c + 1);
		if((c + 1) < 10)	TrackNo.prepend('0');

		TN = TrackNo.text();

		NewGame.GetValue(TN, "name_jp", TYPE_STRING, (void*)TempStr);   	NewTrack->Name[LANG_JP] = TempStr;	TempStr[0] = '\0';
		NewGame.GetValue(TN, "name_en", TYPE_STRING, (void*)TempStr);   	NewTrack->Name[LANG_EN] = TempStr;	TempStr[0] = '\0';

		if(!NewGame.GetValue(TN, "artist", TYPE_STRING, (void*)TempStr))	NewTrack->Artist = GI->Artist;
		else																NewTrack->Artist = TempStr;
		TempStr[0] = '\0';

		NewGame.GetValue(TN, "comment_jp", TYPE_STRING, (void*)TempStr);	NewTrack->Comment[LANG_JP] = TempStr;	TempStr[0] = '\0';	
		NewGame.GetValue(TN, "comment_en", TYPE_STRING, (void*)TempStr);	NewTrack->Comment[LANG_EN] = TempStr;	TempStr[0] = '\0';

		if(!PM[GI->PackMethod]->ParseTrackInfo(NewGame, GI, TN, NewTrack))	continue;

		// Read position data
		// ------------------

		NewGame.GetValue(TN, "start", TYPE_ULONG, (void*)&NewTrack->Start[0]);

		// Absolute values
		NewGame.GetValue(TN, "abs_loop", TYPE_ULONG, (void*)&NewTrack->Loop);
		NewGame.GetValue(TN, "abs_end", TYPE_ULONG, (void*)&NewTrack->End);

		// Relative values
		NewGame.GetValue(TN, "rel_loop", TYPE_ULONG, (void*)&TempVal);	NewTrack->Loop = NewTrack->Start[0] + TempVal;
		NewGame.GetValue(TN, "rel_end", TYPE_ULONG, (void*)&TempVal);	NewTrack->End = NewTrack->Start[0] + TempVal;

		// Coolier array format
		if(NewGame.GetValue(TN, "position", TYPE_STRING, (void*)&TempStr))
		{
			Split.Clear();
			if(SplitString(TempStr, ',', &Split) == 3)
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
			Stat.format("WARNING: Couldn't read track position data for Track #%s%d %s!", (c+1) < 10 ? "0" : "", c+1, NewTrack->Name[Lang]);
			MW->PrintStat(Stat);
		}
	}
	// ---------

	Stat.format("%s (%s)\n", GI->Name[Lang], InfoFile.text());

	MW->PrintStat(Stat);
}

bool ParseDir(FXString Path)
{
	FXString* BGM;
	FXint	BGMCount;

	GameInfo* NewGame;

	MW->PrintStat("---------------------------\nSupported:\n");

	BGMCount = FXDir::listFiles(BGM, Path, "*.bgm");

	if(BGMCount == 0)
	{
		MW->PrintStat("ERROR: No BGM info files found in " + Path + "!\nGet some and then restart this application.\n");
		return false;
	}

	Game.Clear();

	for(FXint c = 0; c < BGMCount; c++)
	{
		NewGame = &(Game.Add()->Data);
		NewGame->Scanned = false;
		ParseGame(BGM[c], NewGame);
	}

	SAFE_DELETE_ARRAY(BGM);

	MW->PrintStat("---------------------------\n\n");

	return true;
}

void PM_Tasofro::GetHeader(GameInfo* GI)
{
	if(GI->Scanned)	return;

	FX::FXFile In;
	FXushort Files;

	FXuint hdrSize;

	char* hdr;

	In.open(GI->BGMFile, FXIO::Reading);

	In.readBlock(&Files, 2);
	hdrSize = GetHeaderSize(In, Files);

	hdr = new char[hdrSize];

	In.readBlock(hdr, hdrSize);
	DecryptBMHeader(hdr, hdrSize);

	// Get track offsets...
	GetPosData(GI, In, Files, hdr, hdrSize);

	SAFE_DELETE_ARRAY(hdr);

	// ... done!
	In.close();

	// GI->Scanned gets set by SilenceScan
}

void SilenceScan(GameInfo* GI)
{
	if(GI->Scanned)	return;

	MW->PrintStat("Scanning silence start values...");

	TrackInfo* TI;
	FX::FXFile In;
	FXulong c;

	bool MultiFile = GI->BGMFile.empty();

	if(!MultiFile)	In.open(GI->BGMFile, FXIO::Reading);

	ListEntry<TrackInfo>* CurTI = GI->Track.First();
	while(CurTI)
	{
		TI = &CurTI->Data;

		if(MultiFile)	In.open(GI->GetTrackFN(TI), FXIO::Reading);

		In.position(TI->Start[0]);
		FXuint BufSize = TI->Loop - TI->Start[0];

		char* Buf = new char[BufSize];
		In.readBlock(Buf, BufSize);

		for(c = 0; c < BufSize; c++)	if(Buf[c] != 0)	break;
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

	MW->PrintStat("Scanning " + Path + "...\n");
	MW->PrintStat("------------------------\n");

	FXSystem::setCurrentDirectory(Path);

	for(char m = BM_COUNT - 1; m > -1; m--)
	{
		if(NewGame = PM[m]->Scan(Path))	break;
	}

	if(NewGame != NULL)
	{
		Message.format("Identified %s\n", NewGame->Name[Lang]);
		MW->PrintStat(Message);
		if(!NewGame->Scanned)	SilenceScan(NewGame);
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

void ReadConfig(const FXString& FN)
{
	ushort Index = 0;
	FXString Sect;
	Encoder* New;
	char Temp[1024];
	ConfigFile Cfg(FN.text());

	Cfg.Load();

	Cfg.GetValue("default", "lang", TYPE_BOOL, &Lang);
	Cfg.GetValue("default", "play", TYPE_BOOL, &Play);
	Cfg.GetValue("default", "showconsole", TYPE_BOOL, &ShowConsole);
	Cfg.GetValue("default", "removesilence", TYPE_BOOL, &SilRem);
	Cfg.GetValue("default", "loop", TYPE_USHORT, &LoopCnt);	LoopCnt = MAX(LoopCnt, 1);
	Cfg.GetValue("default", "fade", TYPE_USHORT, &FadeDur);
	Cfg.GetValue("default", "volume", TYPE_INT, &Volume);
	Cfg.GetValue("default", "enc", TYPE_USHORT, &EncFmt);
	Cfg.GetValue("default", "pattern", TYPE_STRING, (void*)Temp);	FNPattern = Temp;

	Sect.format("enc%d", ++Index);
	while(Cfg.GetValue(Sect.text(), "name", TYPE_STRING, (void*)Temp))
	{
		New = &(Encoders.Add()->Data);
		New->Name = Temp;

		Cfg.GetValue(Sect.text(), "lossless", TYPE_BOOL, (void*)&New->Lossless);
		Cfg.GetValue(Sect.text(), "encoder", TYPE_STRING, (void*)Temp);	New->CmdLine[0] = Temp; New->CmdLine[0].append(" ");
		Cfg.GetValue(Sect.text(), "options", TYPE_STRING, (void*)Temp);	New->CmdLine[1] = Temp;

		Sect.format("enc%d", ++Index);
	}
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
	for(FXint c = 0; c < FN.length(); c++)
	{
		while(FN.at(c) == '?')	FN.erase(c);
	}

	// Append encoder
	FN.append(".");
	FN.append(Encoders.Get(EncFmt - 1)->Data.Name.lower());

	return FN;
}
// -----------------
