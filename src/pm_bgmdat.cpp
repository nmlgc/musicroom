// Touhou Project BGM Extractor
// ----------------------------
// pm_bgmdat.cpp - Parsing for thbgm.dat games (Team Shanghai Alice games starting with th07)
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"

bool PM_BGMDat::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "zwavid_08", TYPE_UCHAR, &GI->ZWAVID[0]);
	NewGame.GetValue("game", "zwavid_09", TYPE_UCHAR, &GI->ZWAVID[1]);
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, &GI->BGMFile);

	PMGame.Add(&GI);
	return true;
}

bool PM_BGMDat::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, const char* TN, TrackInfo *NewTrack)
{
	NewTrack->FN = GI->BGMFile;

	return true;	// Read position info from parsed file
}

// Scanning
// --------
bool PM_BGMDat::CheckZWAVID(GameInfo* Target)
{
	uchar FileID[2];

	FILE* BGM = fopen(Target->BGMFile.text(), "rb");
	fseek(BGM, 8, SEEK_SET);
	fread(&FileID[0], 1, 1, BGM);
	fread(&FileID[1], 1, 1, BGM);
	fclose(BGM);

	return (FileID[0] == Target->ZWAVID[0]) && (FileID[1] == Target->ZWAVID[1]);
}

GameInfo* PM_BGMDat::Scan(FXString& Path)
{
	FXString* Files = NULL;
	FXint FileCount = 0;

	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		FileCount = FXDir::listFiles(Files, Path, CurGame->Data->BGMFile, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
		SAFE_DELETE_ARRAY(Files);

		if(FileCount == 1 && CheckZWAVID(CurGame->Data))
		{
			return CurGame->Data;
		}
		CurGame = CurGame->Next();
	}
	return NULL;
}
// --------
