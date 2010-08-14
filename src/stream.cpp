// Touhou Project BGM Extractor
// ----------------------------
// stream.cpp - Wave streaming
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include "stream.h"

#ifdef WIN32

// Initialization
// --------------
Streamer::Streamer()
{
	Active = false;
	Dev = NULL;
	ZeroMemory(&Fmt, sizeof(WAVEFORMATEX));
	Track = NULL;
	
	Block = NULL;
}

void CALLBACK waveOutProc(HWAVEOUT Dev, uint Msg, ulong Inst, ulong Param1, ulong Param2);

bool Streamer::Init()
{
	MMRESULT Ret;

	MW->PrintStat("Initializing wave streaming...\n");

	// Fill wave format structure
	Fmt.wFormatTag = WAVE_FORMAT_PCM;
	Fmt.nChannels = 2;
	Fmt.nSamplesPerSec = 44100;
	Fmt.wBitsPerSample = 16;
	Fmt.nBlockAlign = (Fmt.wBitsPerSample >> 3) * Fmt.nChannels;
    Fmt.nAvgBytesPerSec = Fmt.nBlockAlign * Fmt.nSamplesPerSec;
	Fmt.cbSize = 0;

	Ret = waveOutOpen(&Dev, WAVE_MAPPER, &Fmt, (DWORD_PTR)waveOutProc, (DWORD_PTR)&FreeBlockCount, CALLBACK_FUNCTION);

	if(Ret != MMSYSERR_NOERROR)
	{
		MW->PrintStat("WARNING: Wave streaming device could not be opened.\nTrack playback won't be available.\n");
	}
	else	Active = true;

	CreateBlocks();

	InitializeCriticalSection(&WaveCS);

	return Active;
}

void Streamer::CreateBlocks()
{
	Block = new WAVEHDR[BlockCount];

	for(uint c = 0; c < BlockCount; c++)
	{
		ZeroMemory(&Block[c], sizeof(WAVEHDR));
		Block[c].dwBufferLength = BlockSize;
		Block[c].lpData = new char[BlockSize];
		Block[c].dwUser = 0;
	}
	FreeBlockCount = BlockCount;
}

void Streamer::UnprepareBlocks()
{
	MMRESULT Ret;
	for(uint c = 0; c < BlockCount; c++)
	{
		Ret = waveOutUnprepareHeader(Dev, &Block[c], sizeof(WAVEHDR));
		ZeroMemory(Block[c].lpData, BlockSize);
	}

	EnterCriticalSection(&WaveCS);
	FreeBlockCount = BlockCount;
	LeaveCriticalSection(&WaveCS);
	CurBlock = 0;
}

void Streamer::ClearBlocks()
{
	UnprepareBlocks();

	for(uint c = 0; c < BlockCount; c++)
	{
		SAFE_DELETE_ARRAY(Block[c].lpData);
	}
	SAFE_DELETE_ARRAY(Block);
}

void Streamer::Exit()
{
	if(!Active)	return;

	Stop();
	CloseFile();

	ClearBlocks();

	DeleteCriticalSection(&WaveCS);
	waveOutClose(Dev);
	MW->PrintStat("Wave streamer closed.\n");
	Active = false;
}
// --------------

// Callback Function
void CALLBACK waveOutProc(HWAVEOUT Dev, uint Msg, ulong Inst, ulong Param1, ulong Param2)
{
	// Check for finished blocks
	if(Msg == WOM_DONE)
	{
		uint* FreeBlockCount = (uint*)Inst;
		EnterCriticalSection(&WaveCS);
		(*FreeBlockCount)++;
		LeaveCriticalSection(&WaveCS);
	}
}

bool Streamer::WriteAudio(char* Raw, int Size)
{
	int Remain; // Remaining bytes in the current buffer
	short s;
	MMRESULT Ret;

	if(FreeBlockCount < 2)
	{
		return false;
	}

	WAVEHDR* CurWave = &Block[CurBlock];

	while(Size > 0)
	{
		// Make sure current header is unprepared
		if(CurWave->dwFlags & WHDR_PREPARED)	waveOutUnprepareHeader(Dev, CurWave, sizeof(WAVEHDR));

		// If raw data is smaller than the remaining size in this block,
		// copy it and return, we only queue full buffers
		if(Size < (int)(BlockSize - CurWave->dwUser))
		{
			memcpy(CurWave->lpData + CurWave->dwUser, Raw, Size);
			CurWave->dwUser += Size;
			return true;
		}

		// Copy only as much raw data to buffer as space is left
		Remain = BlockSize - CurWave->dwUser;

		memcpy(CurWave->lpData + CurWave->dwUser, Raw, Remain);
		CurWave->dwUser += Remain;
		Size -= Remain;
		Raw += Remain;

		VolLog = Volume / 100.0f;

		if(VolLog < 1.0f)
		{
			for(int c = 0; c < BlockSize; c += 2)
			{
				memcpy(&s, &CurWave->lpData[c], sizeof(short));
				s = (float)(s) * VolLog;
				memcpy(&CurWave->lpData[c], &s, sizeof(short));
			}
		}

		// Prepare and send!
		Ret = waveOutPrepareHeader(Dev, CurWave, sizeof(WAVEHDR));
		Ret = waveOutWrite(Dev, CurWave, sizeof(WAVEHDR));

		EnterCriticalSection(&WaveCS);
		FreeBlockCount--;
		LeaveCriticalSection(&WaveCS);

		// Advance to next block
		CurBlock++;
		CurBlock %= BlockCount;
		CurWave = &(Block[CurBlock]);
		CurWave->dwUser = 0;
	}
	return true;
}

void Streamer::StreamFrame_WAV()
{
	if(!Track || !BGMFile)	return;

	static int ReadSize;

	if(FreeBlockCount < 2)	return;
	
	// Read up to the ending point and loop back if necessary
	if( (Pos + ReadBufferSize) >= Track->End)
	{
		ReadSize = Track->End - Pos;
		if(ReadSize != 0)
		{
			fread(ReadBuffer, ReadSize, 1, BGMFile);
		}
	
		if(Track->Loop != Track->End)	Pos = Track->Loop;
		else							Pos = Track->Start[SilRem];
		fseek(BGMFile, Pos, SEEK_SET);
	}
	else
	{
		ReadSize = ReadBufferSize;
		fread(ReadBuffer, ReadSize, 1, BGMFile);
		Pos += ReadSize;
	}

	WriteAudio(ReadBuffer, ReadSize);
}

void Streamer::StreamFrame_OGG()
{
	if(!Track || !BGMFile)	return;

	int Sec;
	long Ret = 1;
	long Rem;

	if(FreeBlockCount < 2)	return;

	if(ov_pcm_tell(&SF) >= Track->End)
	{
		ov_pcm_seek(&SF, Track->Loop);
	}

	Rem = ReadBufferSize;
	while(Rem >= Ret && Ret > 0)
	{
		Ret = ov_read(&SF, &ReadBuffer[ReadBufferSize] - Rem, Rem, 0, 2, 1, &Sec);
		Rem -= Ret;
	}

	WriteAudio(ReadBuffer, ReadBufferSize - Rem);
}


bool Streamer::SwitchTrack_WAV(TrackInfo* NewTrack)
{
	FXString FN = GamePath;

	if(!Track || !BGMFile || (Track->FN != NewTrack->FN) )
	{
		CloseFile();

		FN.append(SlashString);
		FN.append(NewTrack->FN);

		BGMFile = fopen(FN.text(), "rb");
		if(!BGMFile)
		{
			MW->PrintStat("ERROR: Couldn't open " + NewTrack->FN + "!\n");
			return false;
		}
	}
	Pos = NewTrack->Start[SilRem];
	fseek(BGMFile, Pos, SEEK_SET);
	Track = NewTrack;

	return true;
}

bool Streamer::SwitchTrack_OGG(TrackInfo* NewTrack)
{
	FX::FXFile BM;

	if(Track && Track->FN == NewTrack->FN)	return true;

	CloseFile();

	FXString FN = GamePath;
	FN.append(SlashString);
	FN.append(ActiveGame->BGMFile);

	BM.open(FN, FXIO::Reading);
	PM_BMOgg::Inst().DumpOGG(BM, NewTrack->Start[0], NewTrack->FS);
	BM.close();

	BGMFile = fopen(OGGDumpFile.text(), "rb");

	ov_open_callbacks(BGMFile, &SF, NULL, 0, OV_CALLBACKS_DEFAULT);
	
	Track = NewTrack;

	return true;
}

void Streamer::StreamFrame()
{
	if(Track->Vorbis)	return StreamFrame_OGG();
	else				return StreamFrame_WAV();
}

bool Streamer::SwitchTrack(TrackInfo* NewTrack)
{
	Stop();

	if(!NewTrack)
	{
		Track = NULL;
		return true;
	}

	if(NewTrack->Vorbis)	return SwitchTrack_OGG(NewTrack);
	else					return SwitchTrack_WAV(NewTrack);
}

void Streamer::Stop()
{
	waveOutReset(Dev);
	UnprepareBlocks();
}

void Streamer::CloseFile()
{
	if(BGMFile)
	{
		ov_clear(&SF);
		fclose(BGMFile);
		BGMFile = NULL;
		FX::FXFile::remove(OGGDumpFile);
	}
}

#endif