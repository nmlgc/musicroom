// Touhou Project BGM Extractor
// ----------------------------
// extract.cpp - Extracting and encoding functions
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#endif

#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <ogg/xiphcomment.h>
#include <ogg/vorbis/vorbisfile.h>
#include <mpeg/mpegfile.h>
#include <flac/flacfile.h>

#include "extract.h"

const ushort WAV_HEADER_SIZE = 44;
const ulong WAIT_INTERVAL = 20;

// Yes, this is here because I don't want to include taglib anywhere else but in this file
FXString Comment[2];

TagLib::String TLStrConv(FXString x)
{
	return TagLib::String((x).text(), TagLib::String::UTF8);
}

// Vorbis Stuff
// ------------
size_t fread_decrypt(void* _DstBuf, size_t _ElementSize, size_t _Count, FILE* _File)
{
	ulong Pos = ftell(_File);
	char* Out = (char*)_DstBuf;

	size_t Ret = fread(_DstBuf, _ElementSize, _Count, _File);

	PM_BMOgg::Inst().DecryptBuffer(Out, Pos, _Count);

	return Ret;
}

static ov_callbacks OV_CALLBACKS_DECRYPT =
{
	(size_t (*)(void *, size_t, size_t, void *))  fread_decrypt, // Use our own wrapper around fread to decrypt the input stream
	(int (*)(void *, ogg_int64_t, int))           NULL,
	(int (*)(void *))                             fclose,
	(long (*)(void *))                            ftell
};
// ------------

ulong TrackInfo::GetByteLength()
{
	ulong tl, te, len;

	if(Vorbis)
	{
		tl = Loop * 4;
		te = End * 4;
	}
	else
	{
		tl = Loop - Start[SilRem];
		te = End - Start[SilRem];
	}

	if(Loop != 0)
	{
		len = (te - tl) * LoopCnt + tl;
		if(FadeDur > 0 && Loop != End)	len += (ulong)((FadeDur) * 44100.0f * 4.0f);
	}
	else	len = te;

	return len;
}

FXString GameInfo::GetTrackFN(TrackInfo* TI)
{
	FXString Out = GamePath + PATHSEP;
	Out += TI->FN;
	return Out;
}

void makeheader(char *header,int datasize)
{
	int i;
	short s;
	memcpy(header,"RIFF",4);
	i = datasize + 36;
	memcpy(header+4,&i,4);
	memcpy(header+8,"WAVEfmt ",8);
	i = 16;
	memcpy(header+16,&i,4);
	s = 1;
	memcpy(header+20,&s,2);
	s = 2;
	memcpy(header+22,&s,2);
	i = 44100;
	memcpy(header+24,&i,4);
	i = 176400;
	memcpy(header+28,&i,4);
	s = 4;
	memcpy(header+32,&s,2);
	s = 16;
	memcpy(header+34,&s,2);
	memcpy(header+36,"data",4);
	i = datasize;
	memcpy(header+40,&i,4);
}

inline short* EvalFade(short* f, long& c, long& Fade)
{
	*f = (double)*f * (1.0 - ((double)c / (double)Fade));	f++;
	*f = (double)*f * (1.0 - ((double)c / (double)Fade));	f++;
	return f;
}

inline void CalcFade(char* Buf, long& BufSize, long* FadeStart, long* c, long& Fade)
{
	*FadeStart -= BufSize;

	if(*FadeStart <= 0)
	{
		short* f = (short*)&Buf[MAX(BufSize + *FadeStart, 0)];
		for(*c; *c < -(*FadeStart); *c += 4)	f = EvalFade(f, *c, Fade);
	}
}

uint Extractor::ExtractStep1(TrackInfo* TI, FXString& OutFN)
{
	if(!Active)	return Ret;

	FXString Str, Cmd;
	FXString InFN;  	// BGM file to read from (e.g. thbgm.dat)
	Buf = NULL;
	long BufSize, Read;
	long Fade, FadeStart, c = 0;
	short* f;

	// The most special cases...
	// -------------------------
	if( TI->Vorbis && (Ext == "ogg") )
	{
		if( (TI->Loop == 0) || (FadeDur == 0.0f && LoopCnt == 1) )
		{
			PI.hProcess = 0;

			OutFN = PatternFN(TI);
			Str.format("Directly copying %s...", OutFN.text());
			OutFN.prepend(OutPath);
			MW->PrintStat(Str);

			FXString FN = GamePath + SlashString + ActiveGame->BGMFile;

			In.open(FN, FXIO::Reading);
			PM_BMOgg::Inst().DumpOGG(In, TI->Start[0], TI->FS, OGGDumpFile);
			In.close();

			EncFN = OGGDumpFile;

			return ExtractStep2(TI, OutFN);
		}
	}
	Str.format("Creating %s...", OutFN.text());
	// -------------------------

	DumpFN.format("%d.wav", TI->Number);
	EncFN.format("%d.%s", TI->Number, Enc->Name.lower());

	MW->PrintStat(Str);

	char Header[WAV_HEADER_SIZE];
	long Len = TI->GetByteLength();
	makeheader(Header, Len);

	// Load source file
	// ----------------

	if(!TI->Vorbis)
	{
		InFN = ActiveGame->GetTrackFN(TI);
		In.open(InFN, FXIO::Reading);

		In.position(TI->Start[SilRem]);
	}
	else
	{
		MW->PrintStat("decoding...");

		OggVorbis_File VF;
		char pcmout[32768];
		long ret;
		bool eof = false;
		int Sec;

		// Directly decode from the original BGM file
		FXString BGMFN = GamePath + SlashString + ActiveGame->BGMFile;
		FILE* BGM = fopen(BGMFN.text(), "rb");
		fseek(BGM, TI->Start[0], SEEK_SET);

		Out.open(DecodeFile, FXIO::Writing);
		ov_open_callbacks(BGM, &VF, NULL, 0, OV_CALLBACKS_DECRYPT);

		while(!eof)
		{
			ret = ov_read(&VF, pcmout, sizeof(pcmout), 0, 2, 1, &Sec);

			if(ret > 0)	Out.writeBlock(pcmout, ret);
			else		eof = true;
		}

		ov_clear(&VF);

		Out.close();
		fclose(BGM);

		In.open(DecodeFile.text(), FXIO::Reading);
	}
	// ----------------

	// Saves a lot of trouble!
	if(TI->Vorbis)
	{
		TI->Loop *= 4;
		TI->End *= 4;
	}

	Out.open(DumpFN, FXIO::Writing);
	Out.writeBlock(Header, WAV_HEADER_SIZE);

	// Start transfer
	// --------------

	Fade = (ulong)(fabs(FadeDur) * 44100.0f * 4.0f);
	
	if( (TI->Loop == TI->End) || (TI->Loop == 0)) Fade = 0;

	Fade = MIN(Fade, Len);
	FadeStart = Len - Fade;

	// Intro
	if(!TI->Vorbis)	BufSize = TI->Loop - TI->Start[SilRem];
	else	    	BufSize = TI->Loop;

	Buf = new char[BufSize];
	In.readBlock(Buf, BufSize);

	CalcFade(Buf, BufSize, &FadeStart, &c, Fade);

	Out.writeBlock(Buf, BufSize);

	SAFE_DELETE_ARRAY(Buf);

	// Loops
	BufSize = TI->End - TI->Loop;
	Buf = new char[BufSize];

	for(ushort l = 0; l < LoopCnt; l++)
	{
		In.readBlock(Buf, BufSize);

		CalcFade(Buf, BufSize, &FadeStart, &c, Fade);

		Out.writeBlock(Buf, BufSize);
		In.position(TI->Loop);
	}

	SAFE_DELETE_ARRAY(Buf);

	// Fade
	if(Fade != 0)
	{
		ulong Rem;

		Rem = BufSize = Fade - c;

		Buf = new char[BufSize];

		while(Rem > 0)
		{
			Read = MIN(TI->End - TI->Loop, Rem);

			In.readBlock(Buf, Read);
		
			f = (short*)&Buf[0];
			for(c; c < Read; c += 4)	f = EvalFade(f, c, Fade);

			Out.writeBlock(Buf, Read);
			In.position(TI->Loop);

			Rem -= Read;
		}

		SAFE_DELETE_ARRAY(Buf);
	}

	In.close();
	Out.close();

	if(TI->Vorbis)
	{
		TI->Loop /= 4;
		TI->End /= 4;
	}

	// --------------

	// Encode
	// ------
	MW->PrintStat("encoding...");

	Cmd.format("%s%s", AppPath, Enc->CmdLine[0].text());
	Str.format("%s %s \"%s\" \"%s\"", Enc->CmdLine[1], Enc->CmdLine[1], DumpFN, EncFN);
#ifdef WIN32
	// Yes, we have to use this for Unicode compliance!

	STARTUPINFO SI;
	ZeroMemory(&SI, sizeof(STARTUPINFO));
	SI.cb = sizeof(STARTUPINFO);
	SI.dwFlags = STARTF_USESHOWWINDOW;
	SI.wShowWindow = SW_SHOWNOACTIVATE;
	ZeroMemory(&PI, sizeof(PROCESS_INFORMATION));

	wchar_t* CmdW = new FXnchar[Cmd.length() + 1];
	wchar_t* StrW = new FXnchar[Str.length() + 1];

	utf2ncs(CmdW, Cmd.length() + 1, Cmd.text(), Cmd.length() + 1);
	utf2ncs(StrW, Str.length() + 1, Str.text(), Str.length() + 1);

	DWORD Flags = 0;//CREATE_NEW_PROCESS_GROUP;
	if(!ShowConsole)	Flags |= DETACHED_PROCESS;

	BOOL r = CreateProcessW(CmdW, StrW, NULL, NULL, true, Flags, NULL, NULL, &SI, &PI);

	SAFE_DELETE_ARRAY(CmdW);
	SAFE_DELETE_ARRAY(StrW);

	if(!r)
	{
		Str.format("\nERROR: Couldn't start %s!\nPlease make sure this file exists in the directory of this application,\nor reconfigure your encoder settings.\n", Enc->CmdLine[0]);
		MW->PrintStat(Str);
		Ret = MBOX_CLICKED_CANCEL;
		return Cleanup();
	}
#else
	system(Str.text());
#endif
	// ------

	WaitForSingleObject(PI.hProcess, INFINITE);	// Threads are awesome.

	return ExtractStep2(TI, OutFN);
}

uint Extractor::ExtractStep2(TrackInfo* TI, FXString& OutFN)
{
	if(!Active)	return Ret;

	MW->PrintStat("tagging...");
	Tagger::Inst().Tag(TI, EncFN, Ext);

	if(!FXFile::moveFiles(EncFN, OutFN, true))
	{
		FXString Str;
		Str.format("\nERROR: Couldn't write %s!\nDirectory is write-protected, cancelling extraction...\n", OutFN);
		MW->PrintStat(Str);
		Ret = MBOX_CLICKED_CANCEL;
	}
	else	MW->PrintStat("done.\n");

	Cleanup();

	return Ret;
}

uint Extractor::Cleanup()
{
	In.close();
	Out.close();
	FX::FXFile::remove(DumpFN);
	FX::FXFile::remove(EncFN);
	FX::FXFile::remove(DecodeFile);

	return Ret;
}

// Extractor
// ---------

bool Extractor::Start(short ExtStart, short ExtEnd)
{
	FXString Stat;

	Cur = ExtStart;
	Last = ExtEnd;

	Enc = &Encoders.Get(EncFmt - 1)->Data;
	Ext = Enc->Name.lower();

	Stat.format("Extraction started.\nCommand line: %s %s\n-------------------\n", Enc->CmdLine[0], Enc->CmdLine[1]);
	MW->PrintStat(Stat);

	Ret = MBOX_CLICKED_YES;

	cancel();
	start();

	return Active = true;
}

int Extractor::run()
{
	FXString OutFN, Str;
	FXMessageChannel MsgChan(App);

	FXSystem::setCurrentDirectory(FXSystem::getTempDirectory());

	CurTrack = ActiveGame->Track.Get(Cur);
	do
	{
		if(CurTrack->Data.Start[0] == 0)	continue;

		OutFN = OutPath + PatternFN(&CurTrack->Data);
		
		if(FXStat::exists(OutFN))
		{
			if(Ret != MBOX_CLICKED_YESALL && Ret != MBOX_CLICKED_NOALL)
			{
				Str = OutFN + " already exists.\nOverwrite?";
				MsgChan.message(MW, FXSEL(SEL_COMMAND, MW->MW_EXT_MSG), &Str, sizeof(FXString*));
				suspend();
			}
			if(Ret == MBOX_CLICKED_CANCEL)	break;
			if(Ret != MBOX_CLICKED_YES && Ret != MBOX_CLICKED_YESALL)	continue;
		}

		ExtractStep1(&CurTrack->Data, OutFN);
		if(Ret == MBOX_CLICKED_CANCEL)	break;
	}
	while( CurTrack && (CurTrack = CurTrack->Next()) && ++Cur <= Last);

	Finish();

	return 1;
}

// Yay, how stupid.
BOOL CALLBACK EnumWndProc(HWND HWnd, LPARAM lParam)
{
	wchar_t Str[MAX_PATH];
	GetWindowText(HWnd, Str, MAX_PATH);

	FXString Test = Str;
	if(Test.contains(Extractor::Inst().Enc->CmdLine[0]))
	{
		HWND* Trg = (HWND*)lParam;
		*Trg = HWnd;
		return false;
	}
	return true;
}

void Extractor::Stop(FXString* Msg, bool ThreadCall)
{
	if(!Active)	return;

	if(!ThreadCall)	cancel();
	SAFE_DELETE_ARRAY(Buf);

	Active = false;

	if(PI.hProcess != 0)
	{
		HWND EncWnd = NULL;
		EnumWindows(EnumWndProc, (LPARAM)&EncWnd);
		if(EncWnd)
		{
			SendMessage(EncWnd, WM_QUIT, 0, 0);
			SendMessage(EncWnd, WM_CLOSE, 0, 0);
			SendMessage(EncWnd, WM_DESTROY, 0, 0);
		}
	}

	CurTrack = NULL;
	Cur = Last = 0;

	Cleanup();

	MW->handle(this, FXSEL(SEL_COMMAND, MW->MW_EXT_FINISH), NULL);

	if(Msg)	MW->PrintStat(*Msg);
	else	MW->PrintStat("\n-------------------\nExtraction stopped.\n\n");

	detach();
}

bool Extractor::Finish()
{
	if(!Active)	return Active;

	FXString Msg;

	if(Ret != MBOX_CLICKED_CANCEL)	Msg = "-------------------\nExtraction finished.\n\n";
	else                          	Msg = "-------------------\nExtraction canceled.\n\n";

	Stop(&Msg, true);

	return Active;
}
// ---------

// Tagging
// =======

void ID3v2SetCustom(TagLib::ID3v2::Tag* Tag, FXString& Frame, FXString& Value)
{
	FXString OL;
	uint OLLen;
	TagLib::String Conv;

	OL = Frame + '\a' + Value;	// No one will ever use \a in a string, right?
	OLLen = OL.length() + 1;
	OL.substitute('\a', '\0');
	Conv.Copy(OL.text(), OLLen, TagLib::String::UTF8);

	Tag->setTextFrame("TXXX", Conv);
}

bool Tagger::MP3Tag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang)
{
	TagLib::MPEG::File TF(FN);
	if(TF.readOnly())	return false;

	TF.strip();
	TagLib::ID3v2::Tag* Tag = TF.ID3v2Tag(true);
	Tag->removeFrames("TXXX");

	ListEntry<IntString>* Cmp = ActiveGame->Composer.Get(TI->CmpID);

	Tag->setArtist(TLStrConv(Cmp->Data[Lang]));
	ID3v2SetCustom(Tag, FXString("COMPOSER"), Cmp->Data[Lang]);
	Tag->setGenre("Game");
	ID3v2SetCustom(Tag, FXString("ALBUM ARTIST"), ActiveGame->Artist[Lang]);
	ID3v2SetCustom(Tag, FXString("CIRCLE"), ActiveGame->Circle[Lang]);
	ID3v2SetCustom(Tag, FXString("TOTALTRACKS"), FXString::value((FXuint)ActiveGame->Track.Size(), 10));
	
	Tag->setTextFrame("TPOS", TLStrConv(ActiveGame->GameNum));
	Tag->setTrack(TI->Number);
	Tag->setYear(ActiveGame->Year);

	Tag->setTitle(TLStrConv(TI->Name[Lang]));
	Tag->setAlbum(TLStrConv(ActiveGame->FullName(Lang)));
	Tag->setComment(TLStrConv(Comment[Lang]));

	ID3v2SetCustom(Tag, OtherLang + "_CIRCLE", ActiveGame->Circle[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_ARTIST", Cmp->Data[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_NAME", TI->Name[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_COMMENT", Comment[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_GAME", ActiveGame->FullName(!Lang));

	return TF.save();
}

void XiphTag(TagLib::Ogg::XiphComment* Tag, TrackInfo* TI, FXString& OtherLang)
{
	ListEntry<IntString>* Cmp = ActiveGame->Composer.Get(TI->CmpID);

	Tag->setArtist(TLStrConv(Cmp->Data[Lang]));
	Tag->addField(TLStrConv("COMPOSER"), TLStrConv(Cmp->Data[Lang]), true);
	Tag->setGenre("Game");
	Tag->addField(TLStrConv("ALBUM ARTIST"), TLStrConv(ActiveGame->Artist[Lang]), true);
	Tag->addField(TLStrConv("CIRCLE"), TLStrConv(ActiveGame->Circle[Lang]), true);
	Tag->addField(TLStrConv("DISCNUMBER"), TLStrConv(ActiveGame->GameNum), true);
	Tag->addField(TLStrConv("TOTALTRACKS"), TLStrConv(FXString::value((FXuint)ActiveGame->Track.Size(), 10)), true);
	
	Tag->setTrack(TI->Number);
	Tag->setYear(ActiveGame->Year);

	Tag->setTitle(TLStrConv(TI->Name[Lang]));
	Tag->setAlbum(TLStrConv(ActiveGame->FullName(Lang)));
	Tag->addField("COMMENT", TLStrConv(Comment[Lang]));

	Tag->addField(TLStrConv(OtherLang + "_CIRCLE"), TLStrConv(ActiveGame->Circle[!Lang]), true);
	Tag->addField(TLStrConv(OtherLang + "_ARTIST"), TLStrConv(Cmp->Data[!Lang]), true);
	Tag->addField(TLStrConv(OtherLang + "_NAME"), TLStrConv(TI->Name[!Lang]), true);
	Tag->addField(TLStrConv(OtherLang + "_COMMENT"), TLStrConv(Comment[!Lang]), true);
	Tag->addField(TLStrConv(OtherLang + "_GAME"), TLStrConv(ActiveGame->FullName(!Lang)), true);
}

bool Tagger::OGGTag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang)
{
	TagLib::Vorbis::File TF(FN);
	if(TF.readOnly())	return false;

	TagLib::Ogg::XiphComment* Tag = TF.tag();
	XiphTag(Tag, TI, OtherLang);

	return TF.save();
}

bool Tagger::FLACTag(TrackInfo* TI, wchar_t* FN, FXString& OtherLang)
{
	TagLib::FLAC::File TF(FN);
	if(TF.readOnly())	return false;

	TagLib::Ogg::XiphComment* Tag = TF.xiphComment(true);
	XiphTag(Tag, TI, OtherLang);

	return TF.save();
}

bool Tagger::Tag(TrackInfo* TI, FXString& TagFN, FXString& Ext)
{
	FXString OtherLang;
	bool Ret = false;

	// Prepare strings
	// ---------------
	for(ushort c = 0; c < 2; c++)	{Comment[c] = TI->GetComment(c);	Comment[c].substitute("\n", "\r\n");}

	if(Lang == LANG_JP)	OtherLang = "EN";
	else				OtherLang = "JP";
	// ---------------

	wchar_t* WFN = new FXnchar[TagFN.length() + 1];
	utf2ncs(WFN, TagFN.length() + 1, TagFN.text(), TagFN.length() + 1);

	     if(Ext == "flac")	Ret = FLACTag(TI, WFN, OtherLang);
	else if(Ext == "ogg")	Ret = OGGTag(TI, WFN, OtherLang);
	else if(Ext == "mp3")	Ret = MP3Tag(TI, WFN, OtherLang);

	SAFE_DELETE_ARRAY(WFN);

	return Ret;
}
// =======

// Tagger
// ------
bool Tagger::Search(TrackInfo* TI, const FXString& Ext, FXString* FN)
{
	FXString* Files = NULL;
	FXint FileCount = 0;
	wchar_t* WFN = NULL;
	TagLib::File* Test = NULL;
	uint TNo;
	TagLib::String TAlbum;
	bool Ret = false;

	FileCount = FXDir::listFiles(Files, OutPath, "*." + Ext, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
	for(int c = 0; c < FileCount; c++)
	{
		WFN = new FXnchar[Files[c].length() + 1];
		utf2ncs(WFN, Files[c].length() + 1, Files[c].text(), Files[c].length() + 1);

		// I HATE YOU, TAGLIB... just kidding, you're mostly awesome.
		     if(Ext == "flac")	Test = new TagLib::FLAC::File(WFN);
		else if(Ext == "ogg")	Test = new TagLib::Vorbis::File(WFN);
		else if(Ext == "mp3")	Test = new TagLib::MPEG::File(WFN);

		TNo = Test->tag()->track();
		TAlbum = Test->tag()->album();

		if( ((TAlbum == ActiveGame->Name[LANG_JP].text()) || (TAlbum == ActiveGame->Name[LANG_EN].text())) && TNo == TI->Number)
		{
			*FN = Files[c];
			c = FileCount;
			Ret = true;
		}

		SAFE_DELETE(Test);
		SAFE_DELETE_ARRAY(WFN);
	}
	SAFE_DELETE_ARRAY(Files);

	return Ret;
}

FXint Tagger::run()
{
	FXString FN, Stat, Ext;
	TrackInfo* Track;
	StopReq = false;
	Active = true;

	Ext = Encoders.Get(EncFmt - 1)->Data.Name.lower();
	
	Stat.format("Updating tags...\nDirectory: %s\n----------------", OutPath);
	MW->PrintStat(Stat);

	ListEntry<TrackInfo>* CurTrack = ActiveGame->Track.First();

	do
	{
		Track = &CurTrack->Data;
		if(!Track || Track->Start[0] == 0)	continue;

		MW->PrintStat("\n");

		FN = PatternFN(Track);

		if(!FXStat::exists(FN))
		{
			// If someone uses %name_en% and the translation changes, of course we won't find the track anymore *facepalm*
			// Let's try to find it...
			if(Search(Track, Ext, &FN))
			{
				Stat.format("#%d: Tagging %s (found by automatic search)...", Track->Number, FN);
			}
			else
			{
				Stat.format("#%d: ERROR: %s not found!", Track->Number, FN);
				MW->PrintStat(Stat);
				continue;
			}
		}
		else	Stat.format("#%d: Tagging %s...", Track->Number, FN);
		MW->PrintStat(Stat);
		
		if(!Tag(Track, FN, Ext))
		{
			Stat.format("\n%s, tags not written!", WriteError);
			MW->PrintStat(Stat);
		}
	}
	while( (CurTrack = CurTrack->Next()) && !StopReq);

	MW->handle(this, FXSEL(SEL_COMMAND, MW->MW_EXT_FINISH), NULL);

	Stat = "\n----------------\n";
	if(!StopReq)	Stat.append("Updating finished.\n");
	else			Stat.append("Updating stopped.\n");
	MW->PrintStat(Stat);

	StopReq = Active = false;
	detach();

	return 1;
}

void Tagger::Stop()
{
	StopReq = true;
}
// ------
