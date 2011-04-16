// Music Room Interface
// --------------------
// scan.cpp - Track scanning and verification
// --------------------
// "©" Nmlgc, 2010-2011

#include <bgmlib/platform.h>
#include <bgmlib/infostruct.h>
#include <bgmlib/ui.h>
#include <bgmlib/libvorbis.h>
#include <FXFile.h>
#include "scan.h"

// Base class
// ----------
OggVorbis_File	TrackScanner::SF;
FXFile			TrackScanner::F;
FXuint      	TrackScanner::OpenFNHash;	// Comparison hash of [F] and [SF]

bool TrackScanner::Open(GameInfo* GI, TrackInfo* TI)
{
	FXString NewFN = GI->DiskFN(TI);
	FXuint NewHash = NewFN.hash();
	if(NewHash != OpenFNHash)
	{
		Close();
		OpenFNHash = NewHash;
		if(GI->Vorbis)	return OpenVorbisBGM(F, SF, GI, TI);
		else          	return F.open(NewFN);
	}
	return true;
}

ListEntry<TrackInfo>* TrackScanner::Init(GameInfo* GI)
{
	if(!GI)	return NULL;

	ListEntry<TrackInfo>* CurTI = GI->Track.First();
	if(!CurTI)	return NULL;

	if(!Open(GI, &CurTI->Data) || !F.isOpen())	return NULL;

	return CurTI;
}

bool TrackScanner::Close()
{
	ov_clear(&SF);
	F.close();
	OpenFNHash = 0;
	return true;
}
// ----------

// Seek test
// Verifies the number of BGM tracks
// ---------
bool SeekTest::Track(GameInfo* GI, TrackInfo* TI)
{
	Open(GI, TI);
	if(GI->Vorbis)
	{
		ulong te = 1;
		TI->GetPos(FMT_SAMPLE, false, NULL, NULL, &te);
		return (ov_pcm_seek(&SF, te - 1) == 0);
	}
	else
	{
		char Read;

		F.position(TI->GetStart(FMT_BYTE, false));
		return F.readBlock(&Read, 1) == 1;
	}
}

bool SeekTest::Scan(GameInfo* GI)
{
	ushort Seek = 0, Found = 0;
	TrackInfo* TI;
	FXString Str;
	bool Ret = true;
	ListEntry<TrackInfo>* CurTI = Init(GI);

	if(!CurTI)	return false;

	BGMLib::UI_Stat("Verifying track count...");

	do
	{
		TI = &CurTI->Data;
		
		if(TI->GetStart() != 0)
		{
			if(Track(GI, TI))
			{
				Seek = TI->Number;
				Found++;
			}
			else	break;
		}
	}
	while(CurTI = CurTI->Next());

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
	BGMLib::UI_Stat(Str);

	GI->TrackCount = Seek;
	return Ret;
}
// ---------

// Silence scanner
// Finds the amount of leading silence on each track
// ---------------
ulong SilenceScan::Track_PCM(GameInfo *GI, TrackInfo *TI, ulong* Buf, ulong BufSize)
{
	const ulong Comp = 0;
	ulong c;

	F.position(TI->GetStart(FMT_BYTE, false));
	F.readBlock(Buf, BufSize);

	BufSize >>= 2;	// We're comparing in 4-byte steps
	for(c = 0; c < BufSize; c++)
	{
		if(Buf[c] != Comp)	break;
	}
	// Fix IaMP
	if(c == BufSize)	c = 0;
	return c;
}

ulong SilenceScan::Track_Vorbis(GameInfo* GI, TrackInfo* TI, ulong* _Buf, ulong BufSize)
{
	// The naive approach to silence scanning won't work here, because any lossy encoder would smear the previous signal into the silence.
	// So, we are ignoring all audio data at the beginning of the track until we passed a huge block of real, digital silence,
	// and then begin the scanning from there.
	const long Threshold = 1024 * 4;	// around 23 ms
	const char Comp[2] = {-12, 0};
	char b = 0;	// Inner byte loop
	long c = 0, d = 0;	// current section of decoded audio
	long s = 0;	// scanning progress
	long f = 0;	// found bytes
	long Read;
	int Sec;

	char* Buf = (char*)_Buf;
	ov_pcm_seek(&SF, TI->GetStart(FMT_SAMPLE, false));

	BGMLib::UI_Stat(".");

	while(c < (long)BufSize)
	{
		Read = MIN(BufSize - c, Threshold * 2);
		d = ov_read(&SF, Buf + c, Read, 0, 2, 1, &Sec);
		if(d < 0)
		{
			// Something's wrong with the file...
			return false;
		}

		// This code is so brilliant, it gives me an orgasm every time I read it
		for(s; s < (c + d - Threshold); s += 4)
		{
			for(b = 0; b < 4; b++)
			{
				if(!BETWEEN_EQUAL(Buf[s + b], Comp[0], Comp[1]))	break;
			}
			if(b == 4)	f += b;
			else if(f > Threshold)	return s >> 2;
			else					f = 0;
		}

		c += d;
	}
	return 0;
}

ulong SilenceScan::Track(GameInfo* GI, TrackInfo* TI, ulong* Buf, ulong BufSize)
{
	Open(GI, TI);
	if(!GI->Vorbis)	return Track_PCM(GI, TI, Buf, BufSize);
	else			return Track_Vorbis(GI, TI, Buf, BufSize);
}

bool SilenceScan::Scan(GameInfo* GI)
{
	if(GI->Scanned)	return true;
	if(GI->Vorbis && GI->CryptKind)
	{
		return GI->Scanned = true;
	}

	ListEntry<TrackInfo>* CurTI = Init(GI);
	if(!CurTI)	return false;

	BGMLib::UI_Stat("Scanning track start values...");

	TrackInfo* TI;
	ulong* Buf = NULL;	// 32-bit elements
	ulong BufSize;

	ulong ts, tl;

	// LARGE_INTEGER Time[2], Total;
	// QueryPerformanceCounter(&Time[0]);

	for(ushort c = 0; c < GI->TrackCount; c++)
	{
		TI = &CurTI->Data;
		TI->GetPos(FMT_BYTE, false, &ts, &tl);

		// Check for a maximum of 5 seconds
		BufSize = tl - ts;
		BufSize = MIN(BufSize, TI->Freq * 4.0f * 5.0f);
		Buf = (ulong*)realloc(Buf, BufSize);

		ts += Track(GI, TI, Buf, BufSize) << 2;

		if(TI->PosFmt == FMT_SAMPLE)	ts >>= 2;
		TI->Start[1] = ts;
		
		CurTI = CurTI->Next();
	}
	free(Buf);	Buf = NULL;

	// QueryPerformanceCounter(&Time[1]);
	// Total = Time[1] - Time[0];

	BGMLib::UI_Stat("done.\n");
	return true;
}
// ---------------

bool PerformScans(GameInfo* GI)
{
	bool Ret;

	if(GI->Scanned)	return true;

	// Skip seek testing with multiple BGM file games
	if(GI->BGMFile.empty())	Ret = true;
	else					Ret = SeekTest::Inst().Scan(GI);

	if(Ret)
	{
		if(GI->SilenceScan)	Ret = SilenceScan::Inst().Scan(GI);
		GI->Scanned = true;
		TrackScanner::Close();
	}
	return Ret;
}
