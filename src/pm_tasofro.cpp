// Touhou Project BGM Extractor
// ----------------------------
// pm_tasofro.cpp - Parsing for Tasofro archives
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "config.h"
#include "mt.hpp"

// Info
// ----
bool PM_Tasofro::CheckCryptKind(ConfigFile& NewGame, const uchar& CRKind)
{
	if(CRKind == 0 || CRKind > CR_COUNT)
	{
		FXString Str;
		Str.format("ERROR: Invalid encryption kind specified in %s!\n", NewGame.GetFN());
		MW->PrintStat(Str);
		return false;
	}
	return true;
}

bool PM_BMWav::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, &GI->BGMFile);

	PMGame.Add(&GI);
	return CheckCryptKind(NewGame, GI->CryptKind);
}

bool PM_BMWav::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, const char* TN, TrackInfo *NewTrack)
{
	NewGame.GetValue(TN, "filename", TYPE_STRING, &NewTrack->FN);
	NewTrack->Start[0] = GI->HeaderSize;

	return false;	// Read position info from archive
}

bool PM_BMOgg::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, &GI->BGMFile);

	PMGame.Add(&GI);
	return CheckCryptKind(NewGame, GI->CryptKind);
}

bool PM_BMOgg::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, const char* TN, TrackInfo *NewTrack)
{
	NewGame.GetValue(TN, "filename", TYPE_STRING, &NewTrack->FN);
	NewTrack->Vorbis = true;

	return false;	// Read position info from archive
}

// Decryption
// ----------
ulong PM_Tasofro::HeaderSizeV2(FX::FXFile& In)
{
	ulong Ret;
	In.readBlock(&Ret, 4);
	return Ret;
}

bool PM_Tasofro::IsValidHeader(char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	char* p = hdr;
	uchar FNLen;
	for(ushort f = 0; f < Files; f++)
	{
		p += 8;
		memcpy(&FNLen, p, 1); p++;
		p += FNLen;
		if( (p - hdr) > (FXint)hdrSize)	return false;
	}
	return (p - hdr) == hdrSize;
}

void PM_Tasofro::DecryptHeaderV1(char* hdr, const FXuint& hdrSize)
{
	FXushort k = 0x64, l = 0x64;

	for(ushort c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += l; l += 0x4D;
	}
}

void PM_Tasofro::DecryptHeaderV2(char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	RNG_MT mt(hdrSize + 6);
	for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= mt.next_int32() & 0xFF;
	}

	// Check header validity. If we can read the data already, we're done.
	if(IsValidHeader(hdr, hdrSize, Files))	return;

    FXuchar k = 0xC5, t = 0x83;
    for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += t; t +=0x53;
    }
}

// Calling functions

ulong PM_Tasofro::HeaderSize(GameInfo* GI, FX::FXFile& In, const FXushort& Files)
{
	switch(GI->CryptKind)
	{
	case CR_SUIKA:	return Files * GI->EntrySize;
	case CR_TENSHI:	return HeaderSizeV2(In);
	}
	return 0;
}

bool PM_Tasofro::DecryptHeader(GameInfo* GI, char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	switch(GI->CryptKind)
	{
	case CR_SUIKA:	DecryptHeaderV1(hdr, hdrSize);	return true;
	case CR_TENSHI:	DecryptHeaderV2(hdr, hdrSize, Files);	return true;
	}
	return false;
}

// ----------

// Data
// ----
void PM_BMWav::GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize)
{
	char* p = hdr;

	const FXushort hdrJunkSize = GI->EntrySize - 4;

	char* FNTemp = new char[hdrJunkSize];
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
	SAFE_DELETE(FNTemp);
}
// ----

// Data
// ----
void PM_BMOgg::DecryptBuffer(char* Out, ulong Pos, ulong Size)
{
	unsigned char k = (Pos >> 1);
	
	switch(ActiveGame->CryptKind)
	{
	case CR_SUIKA:	k |= 0x08;	break;	// I found that one on my own! I'm really proud of myself :-)
	case CR_TENSHI:	k |= 0x23;	break;
	}

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

bool PM_BMOgg::DumpOGG(FX::FXFile& In, ulong& Pos, ulong& Size, FXString& DumpFN)
{
	FILE* Dec = fopen(DumpFN.text(), "wb");
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
	ulong CFPos = 0, CFSize = 0;
	uchar FNLen;

	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* Track;

	for(ushort c = 0; c < Files; c++)
	{
		if(GI->CryptKind == CR_SUIKA)
		{
			memcpy( &CFPos, p + GI->EntrySize - 4, 4);
			memcpy(&CFSize, p + GI->EntrySize - 8, 4);
			FNLen = strlen(p) - 4;
		}
		else
		{
			memcpy( &CFPos, p, 4);	p += 4;
			memcpy(&CFSize, p, 4);	p += 4;
			memcpy( &FNLen, p, 1);	p += 1;

			FNLen -= 4;
		}

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
		if(GI->CryptKind == CR_SUIKA) p += GI->EntrySize - FNLen - 4;
	}

	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		Track = &CurTrack->Data;
		if(Track->Start[0] == 0)
		{
			Track->FS = 0;
			CurTrack = CurTrack->Next();
			continue;
		}

		if(Track->Loop == 0)
		{
			// Yes, we have to decrypt those and then read them back in to get their sample length...
			OggVorbis_File SF;

			DumpOGG(In, Track->Start[0], Track->FS, OGGDumpFile);
			
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

void PM_Tasofro::TrackData(GameInfo* GI)
{
	FX::FXFile In;
	FXushort Files;

	FXuint hdrSize;

	char* hdr;

	In.open(GI->BGMFile, FXIO::Reading);

	In.readBlock(&Files, 2);
	hdrSize = HeaderSize(GI, In, Files);

	hdr = new char[hdrSize];

	In.readBlock(hdr, hdrSize);
	DecryptHeader(GI, hdr, hdrSize, Files);

	// Get track offsets...
	GetPosData(GI, In, Files, hdr, hdrSize);

	SAFE_DELETE_ARRAY(hdr);

	// ... done!
	In.close();

	// GI->Scanned gets set by SilenceScan
}

// Scanning
// --------

bool PM_BMWav::CheckBMTracks(GameInfo* Target)
{
	FXushort Tracks;
	FX::FXFile In(Target->BGMFile, FXIO::Reading);

	In.readBlock(&Tracks, 2);
	In.close();

	return Tracks == Target->TrackCount;
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

		if(FileCount == 1 && CheckBMTracks(NewGame))	return NewGame;
		CurGame = CurGame->Next();
	}

	return NULL;
}

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

		if(FileCount == 1)	return NewGame;
		CurGame = CurGame->Next();
	}

	return NULL;
}
// --------
