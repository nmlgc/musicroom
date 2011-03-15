// Music Room Interface
// --------------------
// tagger.cpp - Tagging
// --------------------
// "©" Nmlgc, 2010-2011

#include "musicroom.h"
#include <FXFile.h>

#include <FXHash.h>
#include <FXStream.h>
#include <FXObject.h>
#include <FXThread.h>
#include <FXIO.h>
#include <FXDir.h>
#include <FXPath.h>
#include <FXStat.h>

#include <bgmlib/utils.h>
#include <bgmlib/bgmlib.h>
#include <bgmlib/ui.h>
#include <bgmlib/libvorbis.h>

#include "tag_base.h"
#include "tag_id3v2.h"
#include "tag_vorbis.h"
#include "tagger.h"

MRTag* Tagger::NewFmtClass(const FXString& Ext)
{
	FXString Str;

	     if(Ext == "flac")	return new MRTag_Flac;
	else if(Ext == "ogg")	return new MRTag_Ogg;
	else if(Ext == "mp3")	return new MRTag_ID3v2;

	Str.format("\nERROR: Tagging engine doesn't support the %s format!\n", Ext);
	BGMLib::UI_Stat_Safe(Str);
	return NULL;
}

bool Tagger::TagBasic(MRTag* TF, GameInfo* GI, TrackInfo* TI)
{
	ListEntry<IntString>* Cmp;

	if(!TF || !GI || !TI)	return false;

	Cmp = GI->Composer.Get(TI->CmpID);

	// Prepare strings
	// ---------------
	TN[0] = TI->GetNumber();
	TN[1].fromUInt(ActiveGame->Track.Size());
	if(GI->Track.Size() < 10)	TN[1].prepend('0');

	Game[Lang] = GI->FullName(Lang);
	Genre = "Game";
	Year = FXString::value((FXuint)GI->Year, 10);
	// ---------------

	TF->Add(ARTIST, &Cmp->Data[Lang]);
	TF->Add(COMPOSER, &Cmp->Data[Lang]);
	TF->Add(GENRE, &Genre);
	TF->Add(ALBUM_ARTIST, &GI->Artist[Lang]);
	TF->Add(CIRCLE, &GI->Circle[Lang]);
	
	TF->Add(DISCNUMBER, &ActiveGame->GameNum);
	TF->Add(TRACK, &TN[0]);
	TF->Add(TOTALTRACKS, &TN[1]);
	TF->Add(YEAR, &Year);

	TF->Add(TITLE, &TI->Name[Lang]);
	TF->Add(ALBUM, &Game[Lang]);

	return true;
}

bool Tagger::TagExt(MRTag* TF, GameInfo* GI, TrackInfo* TI)
{
	ListEntry<IntString>* Cmp;
	FieldName LB;

	if(!TF || !GI || !TI)	return false;

	Cmp = GI->Composer.Get(TI->CmpID);

	Comment[Lang] = TI->GetComment(Lang);
	Comment[Lang].substitute("\n", "\r\n");

	TF->Add(COMMENT, &Comment[Lang]);

	// i18n
	for(ushort c = 0; c < LANG_COUNT; c++)
	{
		if(c == Lang)	continue;

		LB = (FieldName)(I18N_BASE + (c * I18N_MAX));

		// Strings
		Game[c] = GI->FullName(c);
		Comment[c] = TI->GetComment(c);
		Comment[c].substitute("\n", "\r\n");

		TF->Add(LB + CIRCLE, &GI->Circle[c]);
		TF->Add(LB + ARTIST, &Cmp->Data[c]);
		TF->Add(LB + TITLE, &TI->Name[c]);
		TF->Add(LB + ALBUM, &Game[c]);
		TF->Add(LB + COMMENT, &Comment[c]);
	}
	return true;
}

mrerr Tagger::Tag(TrackInfo* TI, FXString& TagFN, FXString& Ext)
{
	FXString Str;
	mrerr Ret = ERROR_GENERIC;
	MRTag*	TF = NULL;

#ifdef PROFILING_LIBS
	LARGE_INTEGER Time[2], TimeTotal;
#endif

	TF = NewFmtClass(Ext);
	if(!TF)	return ERROR_GENERIC;
	
	Ret = TF->Open(TagFN);
	if(Ret != SUCCESS)	return Ret;
	if(TF->ReadOnly())	return ERROR_FILE_ACCESS;
	TF->ReadTags();

#ifdef PROFILING_LIBS
	QueryPerformanceCounter(&Time[1]);
	TimeTotal = Time[1] - Time[0];
#endif

	TagBasic(TF, ActiveGame, TI);
	TagExt(TF, ActiveGame, TI);

	Ret = TF->Save();
	if(Ret != SUCCESS)
	{
		switch(Ret)
		{
		case ERROR_GENERIC:		Str = "\nError writing tags to file!";
								break;
		case ERROR_FILE_ACCESS:	Str.format("\n%s %s!", BGMLib::WriteError, TagFN);
								break;
		}
		
		BGMLib::UI_Stat_Safe(Str);
	}

	SAFE_DELETE(TF);

	return Ret;
}
// =======

// Tagger
// ------

// Single i18n compare loop
static bool CompareTag(MRTag* Test, const FieldName& ID, const IntString& Src)
{
	char* Tag;
	char Space = ' ';
	FXString Cmp[2];
	
	Tag = Test->Get(ID);
	if(!Tag)	return false;

	// Get rid of that STUPID FULLWIDTH SPACE
	Cmp[0] = Tag;
	Cmp[0].substitute(Moonspace, 3, &Space, 1);

	for(ushort t = 0; t < LANG_COUNT; t++)
	{
		Cmp[1] = Src.s[t];
		Cmp[1].substitute(Moonspace, 3, &Space, 1);
		if(!comparecase(Cmp[0], Cmp[1]) || Cmp[0].contains(Cmp[1]))	return true;
	}
	return false;
}

static bool Compare(MRTag* Test, GameInfo* GI, TrackInfo* TI)
{
	char* Tag;
	FXString Cmp;
	FieldName LB;

	// Normal tags
	if(CompareTag(Test, ALBUM, ActiveGame->Name))	return true;
	if(CompareTag(Test, TITLE, TI->Name))	return true;

	// i18n
	for(ushort l = 0; l < LANG_COUNT; l++)
	{
		LB = (FieldName)(I18N_BASE + (l * I18N_MAX));

		// Album
		// -----
		if( !(Tag = Test->Get(LB + ALBUM)) )	continue;
		Cmp = Tag;
		if(!comparecase(Tag, ActiveGame->Name[l]))	return true;
		// -----

		// Name
		// ----
		if( !(Tag = Test->Get(LB + TITLE)) )	continue;
		Cmp = Tag;
		if(!comparecase(Tag, TI->Name[l]))	return true;
		// ----
	}

	return false;
}

bool Tagger::Search(TrackInfo* TI, const FXString& Ext, FXString* FN)
{
	bool Ret = false;
	FXString* Files = NULL;
	FXint FileCount = 0;
	MRTag* Test = NULL;
	FXuint TNo;
	FXString Cmp;

	FileCount = FXDir::listFiles(Files, OutPath, "*." + Ext, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);

	// Speed up the search by looking for a correctly numbered file
	int c, f = 0;
	Cmp = TI->GetNumber();
	for(c = 0; c < FileCount; c++)	{if(Files[c].contains(Cmp) > 0)	break;}

	Test = NewFmtClass(Ext);
	if(!Test)	return ERROR_GENERIC;
	
	while(f < FileCount)
	{
		if(c == FileCount)	c = 0;

		if(Test->Open(Files[c]) == SUCCESS)
		{
			Test->ReadTags();

			TNo = FXString(Test->Get(TRACK), 2).toUInt();

			if(Compare(Test, ActiveGame, TI) && TNo == TI->Number)
			{
				*FN = Files[c];
				f = FileCount;
				Ret = true;
			}
		}
		Test->Clear();
		f++; c++;
	}
	SAFE_DELETE(Test);
	SAFE_DELETE_ARRAY(Files);

	return Ret;
}

FXint Tagger::run()
{
	FXString FN, Stat, Ext;
	TrackInfo* Track;
	StopReq = false;
	Active = true;

	Ext = Encoders.Get(EncFmt - 1)->Data->Ext.lower();
	
	Stat.format("Updating tags...\nDirectory: %s\n----------------", OutPath);
	BGMLib::UI_Stat_Safe(Stat);

	MRTag::SetDrive(OutPath);

	ListEntry<TrackInfo>* CurTrack = ActiveGame->Track.First();

	do
	{
		Track = &CurTrack->Data;
		if(!Track)	continue;

		BGMLib::UI_Stat_Safe("\n");

		FN = PatternFN(Track);

		if(!FXStat::exists(FN))
		{
			// If someone uses %name_en% and the translation changes, of course we won't find the track anymore *facepalm*
			// Let's try to find it...
			if(Search(Track, Ext, &FN))
			{
				Stat.format("#%s: Tagging %s (found by automatic search)...", Track->GetNumber(), FN);
			}
			else
			{
				Stat.format("#%s: ERROR: %s not found!", Track->GetNumber(), FN);
				BGMLib::UI_Stat_Safe(Stat);
				continue;
			}
		}
		else	Stat.format("#%s: Tagging %s...", Track->GetNumber(), FN);
		BGMLib::UI_Stat_Safe(Stat);
		
		mrerr Ret = Tag(Track, FN, Ext);

		if(Ret != SUCCESS)
		{
			if(Ret == ERROR_FILE_ACCESS)	Stat.format("\n%s, tags not written!", BGMLib::WriteError);
			else							Stat.format("\nERROR: File structure broken or not supported!");
			BGMLib::UI_Stat_Safe(Stat);
		}
	}
	while( (CurTrack = CurTrack->Next()) && !StopReq);

	MW->ActFinish();

	Stat = "\n----------------\n";
	if(!StopReq)	Stat.append("Updating finished.\n");
	else			Stat.append("Updating stopped.\n");
	BGMLib::UI_Stat_Safe(Stat);

	StopReq = Active = false;
	detach();

	return 1;
}

void Tagger::Stop()
{
	if(!Active)	return;

	StopReq = true;
	while(StopReq)	FXThread::sleep(TIMEOUT);
}
// ------
