// Music Room Interface
// --------------------
// tag_base.cpp - Basic definitions for the tagging engine
// --------------------
// "©" Nmlgc, 2011

#include <bgmlib/platform.h>
#include <bgmlib/infostruct.h>
#include <bgmlib/bgmlib.h>
#include <FXFile.h>
#include <FXPath.h>
#include <FXStat.h>
#include <FXSystem.h>
#include "tag_base.h"

#ifdef WIN32
#include <windows.h>
#endif

// FieldName
// ---------
FieldName operator + (const FieldName& a, const FieldName& b)	{int _a = a, _b = b;	return (FieldName)(_a + _b);}
FieldName operator - (const FieldName& a, const FieldName& b)	{int _a = a, _b = b;	return (FieldName)(_a - _b);}
FieldName operator % (const FieldName& a, const FieldName& b)	{int _a = a, _b = b;	return (FieldName)(_a % _b);}

const FXString FNMap_Custom[MAP_END] = {"UNKNOWN", "CIRCLE", "ARTIST", "COMPOSER", "TITLE", "ALBUM", "COMMENT", "ALBUM ARTIST", "TRACKNUMBER", "TOTALTRACKS", "DISCNUMBER", "GENRE", "DATE"};

FXshort FindMapMatch(FXString& In, const FXString* Map, const FXshort& Limit)
{
	if(!In.empty())
	{
		for(FXshort r = 0; r < Limit; r++)	if(!comparecase(In, Map[r]))	return r;
	}
	return -1;
}
// ---------

// Tagging field (only used internally)
// -------------
Field::Field()
{
	Tag = NULL;
	Data = NULL;
	Len = 0;
}

Field::~Field()
{
	SAFE_DELETE_ARRAY(Tag);
}
// -------------

// Tag writer base class (subclassed for the different tagging formats)
// ---------------------
FXuint MRTag::Drive = 0;
ushort MRTag::ClusterSize = 4096;

MRTag::MRTag()	{}

FieldName MRTag::ParseCustom(FXString ID)
{
	FXint s;
	FXString LangStr;
	FieldName Ret = CUSTOM;
	ushort LangID = 0;

	// Try to match first letters of field name (before the '_') to a language
	if( (s = ID.find('_')) != -1)
	{
		LangStr = ID.left(s);
		for(ushort l = 0; l < LANG_COUNT; l++)
		{
			if(!comparecase(LangStr, BGMLib::LI[l].Code2))	LangID = l;
		}
		ID = ID.mid(s + 1, ID.length());	// Remove language part
		Ret = (FieldName)(I18N_BASE + (LangID * I18N_MAX));
	}

	LangID = FindMapMatch(ID, FNMap_Custom, MAP_END);
	if(LangID != -1)	return (FieldName)(Ret + LangID);
	else				return CUSTOM;
}

uint MRTag::RenderField(Field* F)
{
	if(!F)	return 0;
	else if(!F->Data || F->Name == MAP_BEGIN)	return F->Len;

	FXString CustomID;

	// i18n
	// ----
	FieldName Cmp = F->Name;
	if(BETWEEN_EQUAL(Cmp, I18N_BASE, I18N_END))
	{
		Cmp = F->Name - I18N_BASE;	
		ushort l = Cmp / I18N_MAX;	// Language
		Cmp = (FieldName)(Cmp % I18N_MAX);	// Field without language

		CustomID.assign(BGMLib::LI[l].Code2);
		CustomID.upper();
		CustomID.append('_');
		CustomID.append(FNMap_Custom[Cmp]);
	}
	// ----

	return FmtRenderField(F, CustomID);
}

uint MRTag::RenderAll()
{
	ListEntry<Field>* CurTF = TF.First();
	uint Ret = FmtTagHeaderSize();
	while(CurTF)
	{
		Ret += RenderField(&CurTF->Data);
		CurTF = CurTF->Next();
	}
	return Ret;
}

Field* MRTag::Find(const FieldName& Name)
{
	if(Name == CUSTOM)	return NULL;

	ListEntry<Field>* CurTF = TF.First();
	while(CurTF)
	{
		if(CurTF->Data.Name == Name)	return &CurTF->Data;
		CurTF = CurTF->Next();
	}
	return NULL;
}

Field* MRTag::Add(const FieldName& Name, FXString* Data)
{
	if(Data->empty())	return NULL;

	Field* New = Find(Name);
	if(New)
	{
		New->Data = Data;
	}
	else
	{
		New = &TF.Add()->Data;
		New->Name = Name;
		New->Data = Data;
	}
	RenderField(New);
	return New;
}

// Returns the value of the [Name] field
char* MRTag::Get(const FieldName& Name)
{
	Field* F = Find(Name);
	if(!F)	return NULL;
	if(!F->Tag || !F->Len)	return NULL;

	return FmtGet(F);
}

// Sets the cluster size of [Path]'s drive
bool MRTag::SetDrive(const FXString& Path)
{
	uint NDH;
	FXString ND = FXPath::root(Path);

	if(ND.empty())	return false;
	NDH = ND.hash();

	if(NDH != Drive)
	{
		ulong SPC, BPS;

		if(!GetDiskFreeSpaceA(ND.text(), &SPC, &BPS, NULL, NULL))	return false;
		if(!SPC || !BPS)	return false;
		ClusterSize = SPC * BPS;
		Drive = NDH;
	}
	return true;
}

mrerr MRTag::Open(const FXString& _FN)
{
	File.close();

	FN = _FN;
	SetDrive(_FN);

	if(File.open(FN, FXIO::Reading))
	{
		RO = !FXStat::isWritable(FN);
		FTP = FTS = SSP = 0;
		if(!FmtOpen(File, &FTP, &FTS, &SSP))
		{
			File.close();
			return ERROR_GENERIC;
		}
		else	return SUCCESS;
	}
	else	return ERROR_FILE_ACCESS;
}

// Updates <TF> with the tags already present in the loaded file. If this isn't called, all present tags will be removed on writing!
bool MRTag::ReadTags()
{
	if(!File.isOpen())	return false;
	if(FTS == 0)	return false;	// No tags present

	return FmtReadTags(File);
}

// Save all fields to the opened file
mrerr MRTag::Save()
{
	bool Move;	// In case someone overwrites Rewrite
	FXint Rewrite = -1;
	FXint WTS;
	FXFile Out;
	FXint TagSize = -1;
	FXuint FS;

	static const FXString TempFN = FXSystem::getTempDirectory() + SlashString + "bgmlib_tag.tmp";

	if(!File.isOpen())	return ERROR_FILE_ACCESS;

	WTS = RenderAll();
	FS = (File.size() - SSP) + FTP;

	if(WTS > FTS)
	{
		// File padding is not large enough to accomodate new data, write new tag and rewrite audio data
		Rewrite = SSP;

		// Align final file to drive cluster size
		TagSize = ClusterSize - (FS % ClusterSize);
		while(TagSize < WTS)	TagSize += ClusterSize;

		if(!Out.open(TempFN, FXIO::Writing))	return ERROR_FILE_ACCESS;
		Move = true;
	}
	else
	{
		TagSize = FTS;
		if(!Out.open(FN, FXIO::ReadWrite))	return ERROR_FILE_ACCESS;
		Move = false;
	}

	mrerr Ret = FmtSave(Out, TagSize, File, &Rewrite, &FS);

	if(Rewrite != -1)
	{
		File.position(Rewrite);

		FS -= FTP;
		
		char* Stream = new char[FS];
		File.readBlock(Stream, FS);
		Out.writeBlock(Stream, FS);
		SAFE_DELETE_ARRAY(Stream);
	}
	Out.close();
	File.close();

	if(Move)	return (mrerr)FXFile::moveFiles(TempFN, FN, true);
	else		return Ret;
}

void MRTag::Clear()
{
	File.close();
	FN.clear();
	FTP = FTS = SSP = 0;
	TF.Clear();
}

MRTag::~MRTag()
{
	Clear();
}
// ---------------------
