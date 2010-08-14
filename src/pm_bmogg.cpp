// Touhou Project BGM Extractor
// ----------------------------
// pm_bmogg.cpp - Parsing for Brightmoon archives containing encrypted Vorbis files (Tasofro games except th075)
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"
#include "mt.hpp"

void PM_BMOgg::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, (void*)TempStr);	GI->BGMFile = TempStr;	TempStr[0] = '\0';

	PMGame.Add(&GI);
}

bool PM_BMOgg::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, const char* TN, TrackInfo *NewTrack)
{
	NewGame.GetValue(TN, "filename", TYPE_STRING, (void*)TempStr);	NewTrack->FN = TempStr;
	NewTrack->Vorbis = true;

	return false;	// Read position info from archive
}

// Data
// ----
ulong PM_BMOgg::GetHeaderSize(FX::FXFile& In, FXushort& Files)
{
	ulong Ret;
	In.readBlock(&Ret, 4);
	return Ret;
}

void PM_BMOgg::DecryptBMHeader(char* hdr, FXuint hdrSize)
{
	RNG_MT mt(hdrSize + 6);
	for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= mt.next_int32() & 0xFF;
	}

    FXuchar k = 0xC5, t = 0x83;
    for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += t; t +=0x53;
    }
}

void PM_BMOgg::DecryptBuffer(char* Out, ulong Pos, ulong Size)
{
	unsigned char k = (Pos >> 1) | 0x23;
	for(ulong i = 0; i < Size; ++i)
	{
		Out[i] ^= k;
	}
}

char* PM_BMOgg::DecryptFile(FX::FXFile& In, ulong& Pos, ulong& Size)
{
	char* Out = new char[Size];
	if(!Out)	return NULL;

	In.position(Pos);
	In.readBlock(Out, Size);

	DecryptBuffer(Out, Pos, Size);

	return Out;
}

void PM_BMOgg::ReadSFL(FX::FXFile& In, ulong& Pos, ulong& Size, TrackInfo* TI)
{
	char* SFL = DecryptFile(In, Pos, Size);
	char* p = SFL + 28;

	memcpy(&TI->Loop, p, 4); p += 4 + 40;
	memcpy(&TI->End, p, 4);

	TI->End += TI->Loop;

	SAFE_DELETE_ARRAY(SFL);
}

bool PM_BMOgg::DumpOGG(FX::FXFile& In, ulong& Pos, ulong& Size)
{
	FILE* Dec = fopen(OGGDumpFile.text(), "wb");
	if(!Dec)
	{
		MW->PrintStat("ERROR: Couldn't open temporary Vorbis file!\nExtraction from this game won't be possible...\n");
		return false;
	}

	char* DecBuf = DecryptFile(In, Pos, Size);
	fwrite(DecBuf, Size, 1, Dec);
	SAFE_DELETE_ARRAY(DecBuf);

	fclose(Dec);

	return true;
}

void PM_BMOgg::GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize)
{
	char* p = hdr;

	char FNTemp[128];
	FXString FN, FNExt;
	ulong CFPos, CFSize;
	uchar FNLen;

	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* Track;

	for(ushort c = 0; c < Files; c++)
	{
		memcpy( &CFPos, p, 4);	p += 4;
		memcpy(&CFSize, p, 4);	p += 4;
		memcpy( &FNLen, p, 1);	p += 1;

		FNLen -= 4;

		// Split filename in name + extension
		strncpy(FNTemp, p, FNLen);	p += FNLen;
		FNTemp[FNLen] = '\0';	FN = FNTemp;

		strncpy(FNTemp, p, 4);	p += 4;
		FNTemp[4] = '\0';	FNExt = FNTemp;

		CurTrack = GI->Track.First();
		while(CurTrack)
		{
			Track = &CurTrack->Data;

			if((Track->FN) == FN)
			{
				Track->FS = CFSize;
				if(FNExt == ".ogg")
				{
					Track->Start[0] = Track->Start[1] = CFPos;
					Track->FS = CFSize;
				}
				else if(FNExt == ".sfl")
				{
					ReadSFL(In, CFPos, CFSize, Track);
				}
				
				CurTrack = NULL;
			}
			else CurTrack = CurTrack->Next();
		}
	}

	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		Track = &CurTrack->Data;
		if(Track->Loop == 0)
		{
			// Yes, we have to decrypt those and then read them back in to get their sample length...
			OggVorbis_File SF;

			DumpOGG(In, Track->Start[0], Track->FS);
			
			FILE* Dec = fopen(OGGDumpFile.text(), "rb");

			if(ov_open_callbacks(Dec, &SF, NULL, 0, OV_CALLBACKS_DEFAULT))
			{
				sprintf(FNTemp, "Error decrypting %s!\n", Track->FN);
				MW->PrintStat(FNTemp);
			}
			else
			{
				Track->End = ov_pcm_total(&SF, -1);
				Track->Loop = 0;
				ov_clear(&SF);
			}

			fclose(Dec);

			FX::FXFile::remove(OGGDumpFile);
		}
		CurTrack = CurTrack->Next();
	}

	FXSystem::setCurrentDirectory(GamePath);

	// We don't silence scan vorbis files!
	GI->Scanned = true;
}
// ----

// Scanning
// --------

GameInfo* PM_BMOgg::Scan(FXString& Path)
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

		if(FileCount == 1)
		{
			GetHeader(NewGame);
			return NewGame;
		}
		CurGame = CurGame->Next();
	}

	return NULL;
}
// --------
