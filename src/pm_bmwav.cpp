// Touhou Project BGM Extractor
// ----------------------------
// pm_bmwav.cpp - Parsing for Brightmoon archives containing wave files (only th075)
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"

void PM_BMWav::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, (void*)TempStr);	GI->BGMFile = TempStr;	TempStr[0] = '\0';

	PMGame.Add(&GI);
}

bool PM_BMWav::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, const char* TN, TrackInfo *NewTrack)
{
	NewGame.GetValue(TN, "filename", TYPE_STRING, (void*)TempStr);	NewTrack->FN = TempStr;
	NewTrack->Start[0] = GI->HeaderSize;

	return false;	// Read position info from archive
}

// Data
// ----
ulong PM_BMWav::GetHeaderSize(FX::FXFile& In, FXushort& Files)
{
	return Files * 0x6C;
}

void PM_BMWav::DecryptBMHeader(char* hdr, FXuint hdrSize)
{
	FXushort k = 0x64, l = 0x64;

	for(ushort c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += l; l += 0x4D;
	}
}

void PM_BMWav::GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize)
{
	char* p = hdr;

	const FXushort hdrJunkSize = 0x68;
	const FXushort EntrySize = 0x6C;

	char FNTemp[hdrJunkSize];
	FXString FN;

	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* Track;

	FXuint DataSize;

	for(ushort c = 0; c < Files; c++)
	{
		strcpy(FNTemp, p);	FN = FNTemp;
		p += hdrJunkSize;

		CurTrack = GI->Track.First();
		while(CurTrack)
		{
			Track = &CurTrack->Data;

			if((Track->FN + ".wav") == FN)
			{
				memcpy(&Track->Start[0], p, 4);
				p += 4;
				Track->FN = GI->BGMFile;
				CurTrack = NULL;
			}
			else	CurTrack = CurTrack->Next();
		}
	}

	// Get loop and end data...
	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		Track = &CurTrack->Data;

		In.position(Track->Start[0] + 40);
		In.readBlock(&DataSize, 4);	// Wave data size

		In.position(DataSize + 16, FXIO::Current);

		In.readBlock(&Track->Loop, 4);	// Loop in samples

		In.position(40, FXIO::Current);
		In.readBlock(&Track->End, 4);	// End in samples, relative to loop

		Track->Loop *= 4;
		Track->End *= 4;
		Track->End += Track->Loop;

		Track->Start[0] += GI->HeaderSize;
		Track->End += Track->Start[0];
		Track->Loop += Track->Start[0];

		if(Track->Loop == Track->Start[0])	Track->Loop = Track->End;

		CurTrack = CurTrack->Next();
	}
}
// ----

// Scanning
// --------

bool PM_BMWav::CheckBMTracks(GameInfo* Target)
{
	FXushort Tracks;
	FX::FXFile In(Target->BGMFile, FXIO::Reading);

	In.readBlock(&Tracks, 2);
	In.close();

	return Tracks == Target->Track.Size();
}

GameInfo* PM_BMWav::Scan(FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString* Files = NULL;
	FXint FileCount = 0;
	
	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		NewGame = CurGame->Data;

		FileCount = FXDir::listFiles(Files, Path, NewGame->BGMFile, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
		SAFE_DELETE_ARRAY(Files);

		if(FileCount == 1 && CheckBMTracks(NewGame))
		{
			GetHeader(NewGame);
			return NewGame;
		}
		CurGame = CurGame->Next();
	}

	return NULL;
}
// --------
