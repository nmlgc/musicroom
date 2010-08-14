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

const ushort WAV_HEADER_SIZE = 44;

// Globals
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

// Tagging
// =======

void ID3v2SetCustom(TagLib::ID3v2::Tag* Tag, FXString& Frame, FXString& Value)
{
	FXString OL;
	uint OLLen;
	TagLib::String Conv;

	OL = Frame + '\r' + Value;
	OLLen = OL.length() + 1;
	OL.substitute('\r', '\0');
	Conv.Copy(OL.text(), OLLen, TagLib::String::UTF8);

	Tag->setTextFrame("TXXX", Conv);
}

void MP3Tag(TrackInfo* TI, FXString& FN, FXString& OtherLang)
{
	MW->PrintStat("tagging...");

	TagLib::MPEG::File TF(FN.text());

	TagLib::ID3v2::Tag* Tag = TF.ID3v2Tag(true);

	Tag->setArtist(TLStrConv(TI->Artist));
	ID3v2SetCustom(Tag, FXString("COMPOSER"), TI->Artist);
	Tag->setGenre("Game");
	ID3v2SetCustom(Tag, FXString("ALBUM ARTIST"), ActiveGame->Artist);
	ID3v2SetCustom(Tag, FXString("TOTALTRACKS"), FXString::value((FXuint)ActiveGame->Track.Size(), 10));
	
	Tag->setTextFrame("TPOS", TLStrConv(ActiveGame->GameNum));
	Tag->setTrack(TI->Number);
	Tag->setYear(ActiveGame->Year);

	Tag->setTitle(TLStrConv(TI->Name[Lang]));
	Tag->setAlbum(TLStrConv(ActiveGame->Name[Lang]));
	Tag->setComment(TLStrConv(Comment[Lang]));

	ID3v2SetCustom(Tag, OtherLang + "_NAME", TI->Name[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_COMMENT", Comment[!Lang]);
	ID3v2SetCustom(Tag, OtherLang + "_GAME", ActiveGame->Name[!Lang]);

	TF.save();
}

void XiphTag(TagLib::Ogg::XiphComment* Tag, TrackInfo* TI, FXString& OtherLang)
{
	Tag->setArtist(TLStrConv(TI->Artist));
	Tag->addField(TLStrConv("COMPOSER"), TLStrConv(TI->Artist), false);
	Tag->setGenre("Game");
	Tag->addField(TLStrConv("ALBUM ARTIST"), TLStrConv(ActiveGame->Artist), false);
	Tag->addField(TLStrConv("DISCNUMBER"), TLStrConv(ActiveGame->GameNum), false);
	Tag->addField(TLStrConv("TOTALTRACKS"), TLStrConv(FXString::value((FXuint)ActiveGame->Track.Size(), 10)), false);
	
	Tag->setTrack(TI->Number);
	Tag->setYear(ActiveGame->Year);

	Tag->setTitle(TLStrConv(TI->Name[Lang]));
	Tag->setAlbum(TLStrConv(ActiveGame->Name[Lang]));
	Tag->addField("COMMENT", TLStrConv(Comment[Lang]));

	Tag->addField(TLStrConv(OtherLang + "_NAME"), TLStrConv(TI->Name[!Lang]), false);
	Tag->addField(TLStrConv(OtherLang + "_COMMENT"), TLStrConv(Comment[!Lang]), false);
	Tag->addField(TLStrConv(OtherLang + "_GAME"), TLStrConv(ActiveGame->Name[!Lang]), false);
}

void OGGTag(TrackInfo* TI, FXString& FN, FXString& OtherLang)
{
	MW->PrintStat("tagging...");

	TagLib::Vorbis::File TF(FN.text());

	TagLib::Ogg::XiphComment* Tag = TF.tag();
	XiphTag(Tag, TI, OtherLang);

	TF.save();
}

void FLACTag(TrackInfo* TI, FXString& FN, FXString& OtherLang)
{
	MW->PrintStat("tagging...");

	TagLib::FLAC::File TF(FN.text());

	TagLib::Ogg::XiphComment* Tag = TF.xiphComment(true);
	XiphTag(Tag, TI, OtherLang);

	TF.save();
}
// =======

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
		if(FadeDur > 0 && Loop != End)	len += (FadeDur) * 44100 * 4;
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

uint ExtractTrack(TrackInfo* TI)
{
	FXString Str, Cmd;
	FXString InFN;  	// BGM file to read from (e.g. thbgm.dat)
	FXString DumpFN;	// Temporary extraction wave file, created in a temporary directory (e.g. extract.wav)
	FXString EncFN; 	// Temporary path to the encoded file in the temporary directory (e.g. extract.mp3)
	FXString OutFN;		// Final encoded output file
	FX::FXFile In;
	char* Buf = NULL;
	long BufSize, Read;
	long Fade, FadeStart, c = 0;
	short* f;
	static uint Ret = MBOX_CLICKED_YES;
	Encoder* Enc = &Encoders.Get(EncFmt - 1)->Data;
	FXString Ext = Enc->Name.lower();

	// OutFN
	// -----

	OutFN = PatternFN(TI);
	Str.format("Creating %s...", OutFN.text());
	OutFN.prepend(OutPath);
	// ------------------------

	if(Ret != MBOX_CLICKED_YESALL && Ret != MBOX_CLICKED_NOALL)
	{
		if(FXStat::exists(OutFN))	Ret = FXMessageBox::question(MW->getApp(), MBOX_YES_YESALL_NO_NOALL_CANCEL, PrgName.text(), "%s already exists.\nOverwrite?", OutFN);
	}

	if(Ret != MBOX_CLICKED_YES && Ret != MBOX_CLICKED_YESALL)	return Ret;
	// -----

	// The most special cases...
	// -------------------------
	if( TI->Vorbis && (Ext == "ogg") )
	{
		if( (TI->Loop == 0) || (FadeDur == 0 && LoopCnt == 1) )
		{
			OutFN = PatternFN(TI);
			Str.format("Directly copying %s...", OutFN.text());
			OutFN.prepend(OutPath);
			MW->PrintStat(Str);

			FXString FN = GamePath + SlashString + ActiveGame->BGMFile;

			In.open(FN, FXIO::Reading);
			PM_BMOgg::Inst().DumpOGG(In, TI->Start[0], TI->FS);
			In.close();

			TagTrack(TI, OGGDumpFile, Ext);

			FX::FXFile::moveFiles(OGGDumpFile, OutFN, true);

			MW->PrintStat("done.\n");

			return Ret;
		}
	}
	// -------------------------

	FXSystem::setCurrentDirectory(FXSystem::getTempDirectory());
	
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

		FILE* Decode = fopen(DecodeFile.text(), "wb");

		ov_open_callbacks(BGM, &VF, NULL, 0, OV_CALLBACKS_DECRYPT);

		while(!eof)
		{
			ret = ov_read(&VF, pcmout, sizeof(pcmout), 0, 2, 1, &Sec);

			if(ret > 0)	fwrite(pcmout, ret, 1, Decode);
			else		eof = true;
		}

		ov_clear(&VF);

		fclose(Decode);
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

	FX::FXFile Out(DumpFN, FXIO::Writing);
	Out.writeBlock(Header, WAV_HEADER_SIZE);

	// Start transfer
	// --------------

	Fade = abs(FadeDur) * 44100 * 4;
	
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
	STARTUPINFO SI;
	ZeroMemory(&SI, sizeof(STARTUPINFO));
	SI.cb = sizeof(STARTUPINFO);
	SI.dwFlags = STARTF_USESHOWWINDOW;
	SI.wShowWindow = SW_SHOWNOACTIVATE;

	PROCESS_INFORMATION PI;
	ZeroMemory(&PI, sizeof(PROCESS_INFORMATION));

	wchar_t* CmdW = new FXnchar[Cmd.length() + 1];
	wchar_t* StrW = new FXnchar[Str.length() + 1];

	utf2ncs(CmdW, Cmd.length() + 1, Cmd.text(), Cmd.length() + 1);
	utf2ncs(StrW, Str.length() + 1, Str.text(), Str.length() + 1);

	if(!CreateProcessW(CmdW, StrW, NULL, NULL, false, ShowConsole ? 0 : DETACHED_PROCESS, NULL, NULL, &SI, &PI))
	{
		Str.format("\nERROR: Couldn't start %s!\nPlease make sure this file exists in the directory of this application,\nor reconfigure your encoder settings.\n", Enc->CmdLine[0]);
		MW->PrintStat(Str);
		Ret = MBOX_CLICKED_CANCEL;
		goto Cleanup;
	}
	else	WaitForSingleObject(PI.hProcess, INFINITE);
#else
	system(Str.text());
#endif
	// ------

	TagTrack(TI, EncFN, Ext);

	// Move
	// ====
	FX::FXFile::moveFiles(EncFN, OutFN, true);
	// ====

	MW->PrintStat("done.\n");

Cleanup:
	SAFE_DELETE_ARRAY(CmdW);
	SAFE_DELETE_ARRAY(StrW);
	
	FX::FXFile::remove(DumpFN);
	FX::FXFile::remove(EncFN);
	FX::FXFile::remove(DecodeFile);

	return Ret;
}

void TagTrack(TrackInfo* TI, FXString& TagFN, FXString& Ext)
{
	FXString OtherLang;

	// Prepare strings
	// ---------------
	for(ushort c = 0; c < 2; c++)	{Comment[c] = TI->Comment[c];	Comment[c].substitute("\n", "\r\n");}

	if(Lang == LANG_JP)	OtherLang = "EN";
	else				OtherLang = "JP";
	// ---------------

	     if(Ext == "flac")	FLACTag(TI, TagFN, OtherLang);
	else if(Ext == "mp3")	MP3Tag(TI, TagFN, OtherLang);
	else if(Ext == "ogg")	OGGTag(TI, TagFN, OtherLang);
}
