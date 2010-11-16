// Legacy Engine
// -------------
// config.cpp - Parsing structures for simple cross-platform reading and writing of configuration files
// -------------
// "©" Nameless, 2008-2009

#include "thbgmext.h"
#include "list.h"
#include "config.h"

// Utils
// -----

#ifdef WIN32
#include <ctype.h>
#endif

// Cross platform text-mode line reading function
int ReadLineFromFile(char* String, int MaxChars, FILE* File)
{
	int c;
	int Len = MaxChars;

	for(c = 0; c < MaxChars - 1; c++)
	{
		if(!fread(String + c, 1, 1, File))
		{
			if(c == 0)	return -1;
			else
			{
				// Reached EOF, terminate string
				String[c++] = '\0';
				break;
			}
		};

		if(String[c] == LineBreak[0])
		{
			String[c] = '\0';
			Len = MIN(Len, c + 1);
		}
		else if(String[c] == LineBreak[1])
		{
			String[c] = '\0';
			Len = MIN(Len, c + 1);
			break;
		}
	}
	return Len;
}

// Transforms a string to lowercase
bool LowerString(char* Dest, const char* Source)
{
	if(!Source || !Dest) return false;

	bool Lock = false;

	ushort Length = strlen(Source) + 1;
	for(ushort c = 0; c < Length; c++)
	{
		if(Source[c] == '"')	Lock = !Lock;

		// This function isn't used for any other purpose, so we can safely do this...
		if(Source[c] == '=' || Source[c] == '#')
		{
			strcpy(&Dest[c], &Source[c]);
			return true;
		}

		if(!Lock)	Dest[c] = tolower(Source[c]);
		else		Dest[c] = Source[c];
	}
	return true;
}

ulong SplitString(const char* String, const char Delimiter, PList<char>* Result)
{
	if(!Result || !String || !Delimiter)	return 0;

	char* TempString = new char[strlen(String) + 1];
	strcpy(TempString, String);
	char* CurToken = strtok(TempString, &Delimiter);

	while(CurToken)
	{
		Result->Add(CurToken, strlen(CurToken) + 1);
		CurToken = strtok(NULL, &Delimiter);
	}
	SAFE_DELETE_ARRAY(TempString);
	return Result->Size();
}
// -----

// ConfigKey
// ---------
ConfigKey::ConfigKey()
{
	Data = NULL;
	DataType = 0;
}

ConfigKey::ConfigKey(FXString& _Key_, void* _Data_, ushort _DataType_)
{
	SetInfo(_Key_,  _Data_, _DataType_);
}

void ConfigKey::Link(void* _Data_, ushort _DataType_)
{
	Data = _Data_;
	DataType = _DataType_;
	GetData(Data, DataType);
}

void ConfigKey::SetInfo(FXString& _Key_, void* _Data_, ushort _DataType_)
{
	if(!_Key_ || !_Data_)	return;

	Key = _Key_.lower();
	Data = _Data_;
	DataType = _DataType_;
}

FXint BaseCheck(FXString* Str)
{
	FXint Base = 10;
	if(Str->left(2) == "0x")
	{
		Str->erase(0, 2);
		Base = 16;
	}
	return Base;
}

bool ConfigKey::GetData(void* p, ushort pd, FXString* NewLine)
{
	if(!p || pd == 0)	return false;

	FXString* L = NewLine ? NewLine : &Line;

	switch(pd)
	{
	case TYPE_BOOL:	
		if(*L == "true")		*(bool*)p = true;
		else if(*L == "false")	*(bool*)p = false;
		else                    *(bool*)p = atoi(L->text()) != 0;
		break;

	case TYPE_SHORT:	*(short*)p = (short)L->toInt(BaseCheck(L));		break;
	case TYPE_USHORT:	*(ushort*)p = (ushort)L->toUInt(BaseCheck(L));	break;
	case TYPE_INT:		*(int*)p = L->toInt(BaseCheck(L));				break;
	case TYPE_UINT:		*(uint*)p = L->toUInt(BaseCheck(L));            break;
	case TYPE_LONG:		*(long*)p = L->toLong(BaseCheck(L));            break;
	case TYPE_ULONG:	*(ulong*)p = L->toULong(BaseCheck(L));			break;
	case TYPE_FLOAT:	*(float*)p = (float)L->toFloat();				break;
	case TYPE_UCHAR:	*(uchar*)p = (uchar)L->toUInt(BaseCheck(L));    break;
	case TYPE_STRING:
		L->substitute("\\n", 2, &LineBreak[1], 1, true);	// Translate line breaks
		L->substitute("\\\"", 2, "\"", 1, true);
		// Remove quotation marks
		FXint len = L->length() - 1;
		if(L->at(0) == '"' && L->at(len) == '"')	(*(FXString*)p) = L->mid(1, len - 1);
		else (*(FXString*)p) = *L;
		break;
	}
	return true;
}

bool ConfigKey::SaveData(FXString* FileLine)
{
	if(!Data || DataType == 0 || !FileLine)	return false;

	switch(DataType)
	{
	case TYPE_BOOL:
		if(*(bool*)Data)  FileLine->format("%s = true", Key);
		else		  FileLine->format("%s = false", Key);
		break;

	case TYPE_SHORT:	FileLine->format("%s = %d", Key, *(short*)Data);   break;
	case TYPE_USHORT:	FileLine->format("%s = %d", Key, *(ushort*)Data);  break;
	case TYPE_INT:		FileLine->format("%s = %d", Key, *(int*)Data);     break;
	case TYPE_UINT:		FileLine->format("%s = %d", Key, *(uint*)Data);    break;
	case TYPE_LONG:		FileLine->format("%s = %ld",Key, *(long*)Data);    break;
	case TYPE_ULONG:	FileLine->format("%s = %lu",Key, *(ulong*)Data);   break;
	case TYPE_FLOAT:	FileLine->format("%s = %f", Key, *(float*)Data);   break;
	case TYPE_STRING:
		*FileLine = *(FXString*)Data;
		FileLine->substitute("\"", 1, "\\\"", 2, true);	// Enquote
		FileLine->append('\"');
		FileLine->prepend(Key + " = \"");
		FileLine->substitute(&LineBreak[1], 1, "\\n", 2, true);	// Translate line breaks
		break;
	}
	return true;
}
// ConfigParser
// ------------
ConfigParser::ConfigParser()
{
}

void ConfigParser::SetCaption(FXString& New)
{
	Caption = New.lower();
}

#define ADD_KEY_IMP(tn, tc)	ConfigKey* ConfigParser::AddKey(FXString& Key, tn* Data)   {ConfigKey NewKey; NewKey.SetInfo(Key, (void*)Data, tc); return &Keys.Add(&NewKey)->Data;}

ADD_KEY_IMP(bool, TYPE_BOOL)
ADD_KEY_IMP(short, TYPE_SHORT)
ADD_KEY_IMP(ushort, TYPE_USHORT)
ADD_KEY_IMP(int, TYPE_INT)
ADD_KEY_IMP(uint, TYPE_UINT)
ADD_KEY_IMP(long, TYPE_LONG)
ADD_KEY_IMP(ulong, TYPE_ULONG)
ADD_KEY_IMP(float, TYPE_FLOAT)
ADD_KEY_IMP(FXString, TYPE_STRING)

ConfigKey* ConfigParser::FindKey(char* Name)
{
	ListEntry<ConfigKey>* CurKey = Keys.First();

	while(CurKey)
	{
		if(CurKey->Data.Key == Name)	return &CurKey->Data;
		CurKey = CurKey->Next();
	}
	return NULL;
}

void ConfigParser::Clear()
{
	Keys.Clear();
}

ConfigParser::~ConfigParser()
{
	Clear();
}
// ------------

// ConfigFile
// ----------
void ConfigFile::SetFN(const char* FN)
{
	SAFE_DELETE_ARRAY(ConfigFN);
	ConfigFN = new char[strlen(FN) + 1];
	strcpy(ConfigFN, FN);
}

bool ConfigFile::BufferFile()
{
	char CurLine[2048], NewLine[2048];
	int Len;

	FILE* Config = fopen(ConfigFN, "rb");
	if(!Config) return false;//Log.Error("Config file not found!");

	// Skip byte order mark, if necessary
	fread(&CurLine, 3, 1, Config);
	if(memcmp(CurLine, utf8bom, 3))	fseek(Config, 0, SEEK_SET);

	while( (Len = ReadLineFromFile(CurLine, 2048 * sizeof(char), Config)) != -1)
	{
		if(CurLine[0] == '\0')	FileBuffer.Add(CurLine, 0);
		else if(CurLine[0] != '#')
		{
			LowerString(NewLine, CurLine);
			FileBuffer.Add(NewLine, Len);
		}
		else	FileBuffer.Add(CurLine, Len);
		CurLine[0] = '\0';
	}
	fclose(Config);
	return true;
}

PListEntry<char>* ConfigFile::InsertLine(PListEntry<char>* PrevLine, char* NewLine)
{
	if(PrevLine)
	{
		PrevLine = FileBuffer.Insert(PrevLine, NewLine, 256);
	}
	else	FileBuffer.Add(NewLine, strlen(NewLine + 1));
	return PrevLine;
}

ConfigParser* ConfigFile::FindSection(const char* Name)
{
	ListEntry<ConfigParser>* Cur = Sect.First();

	while(Cur)
	{
		if(Cur->Data.Caption == Name)	return &Cur->Data;
		Cur = Cur->Next();
	}

	return NULL;
}

ConfigParser* ConfigFile::CheckSection(char* Line)
{
	char* Temp;

	Temp = new char[strlen(Line)];

	strcpy(Temp, Line + 1);
	Temp[strlen(Temp) - 1] = '\0';

	ConfigParser* NewSection = FindSection(Temp);
	if(!NewSection)
	{
		// Create new section
		NewSection = &(Sect.Add()->Data);
		NewSection->Caption = Temp;
	}

	SAFE_DELETE_ARRAY(Temp);
	return NewSection;
}

bool ConfigFile::Load(const char* FN)
{
	SetFN(FN);
	if(!BufferFile())	return false;
	Load();
	return true;
}

void ConfigFile::Load()
{
	char*         CurLine = NULL;
	ConfigParser* CurSection = NULL;

	char	Key[32];
	char	Equals[32];
	FXString	Value;

	ConfigKey*	TrgKey;

	PList<char> Split;
	PListEntry<char>* CurSS;

	int CurLineNum = -1;
	LineLink* NewLL;

	PListEntry<char>* BufferLine = FileBuffer.First();
	if(!BufferLine)	return;

	do
	{
		CurLineNum++;
		CurLine = BufferLine->Data;
		if(!CurLine)	continue;

		// Comments
		     if(CurLine[0] == '#')	 continue;
		else if(CurLine[0] == '[')	// Section Name
		{
			CurSection = CheckSection(CurLine);
			continue;
		}

		if(!CurSection)	continue;

		// Scanning...
		Split.Clear();
		SplitString(CurLine, ' ', &Split);

		if(!(CurSS = Split.First()))	continue;

		strcpy(Key, CurSS->Data);   	if(!(CurSS = CurSS->Next()))	continue;
		strcpy(Equals, CurSS->Data);	if(!(CurSS = CurSS->Next()))	continue;
		if(CurSS == Split.Last())	Value = CurSS->Data;
		else
		{
			Value.clear();
			while(CurSS)
			{
				Value.append(CurSS->Data);
				if(CurSS != Split.Last())	Value.append(" ");
				CurSS = CurSS->Next();
			}
		}

		if(Equals[0] == '=')
		{
			TrgKey = CurSection->FindKey(Key);
					
			if(!TrgKey)
			{
				TrgKey = &(CurSection->Keys.Add()->Data);
				TrgKey->Key = Key;
				TrgKey->Line = Value;
			}
			else TrgKey->GetData(TrgKey->Data, TrgKey->DataType);

			NewLL = &(Link.Add()->Data);
			NewLL->Key = TrgKey;
			NewLL->Line = CurLineNum;
		}
	}
	while(BufferLine = BufferLine->Next());
}

// Why wasn't I implementing something like this in the first place?
// Simple, fast and bullshit-free saving at the cost of 16 bytes per key.

bool ConfigFile::Save()
{
	PListEntry<char>* BufferLine = FileBuffer.First();	if(!BufferLine)	return false;
	ListEntry<LineLink>* CurLL = Link.First();	if(!CurLL)	return false;

	char* CurLine = NULL;
	LineLink* LL;
	int CurLineNum = -1;

	bool Changed = false;

	FXString Save;

	do
	{
		CurLineNum++;
		if(CurLL->Data.Line != CurLineNum)	continue;

		LL = &CurLL->Data;
		CurLL = CurLL->Next();

		CurLine = BufferLine->Data;
		if(!CurLine)	continue;

		if(LL->Key->SaveData(&Save) && Save != BufferLine->Data)
		{
			BufferLine->SetData(Save.text(), Save.length() + 1);
			Changed = true;
		}
	}
	while(( BufferLine = BufferLine->Next()) && CurLL);

	if(Changed)	return WriteBuffer();	// No changes, no save.
	return Changed;
}

ConfigKey* ConfigFile::FindKey(const char* SectStr, char* KeyStr)
{
	ConfigParser* Section = FindSection(SectStr);
	if(!Section)	return NULL;

	return Section->FindKey(KeyStr);
}

bool ConfigFile::GetValue(const char* SectStr, char* KeyStr, ushort DataType, void* Value)
{
	ConfigKey* Key = FindKey(SectStr, KeyStr);
	if(!Key)	return false;

	Key->GetData(Value, DataType);
	return true;
}

bool ConfigFile::LinkValue(const char* SectStr, char* KeyStr, ushort DataType, void* Value)
{
	ConfigKey* Key = FindKey(SectStr, KeyStr);
	if(!Key)	return false;

	Key->Link(Value, DataType);
	return true;
}

bool ConfigFile::WriteBuffer()
{
	PListEntry<char>* CurLine = FileBuffer.First();
	if(!CurLine) return false;

	FILE* Config = fopen(OGGDumpFile.text(), "wb");
	if(!Config)	return false;

	// Always write UTF8
	fwrite(utf8bom, 3, 1, Config);

	// We have to do it this way, unless we want to append a line each time we save the file.
	fwrite(CurLine->Data, 1, strlen(CurLine->Data), Config);
	CurLine = CurLine->Next();
	while(CurLine)
	{
		fwrite(LineBreak, 1, sizeof(LineBreak), Config);
		if(CurLine->Data)	fwrite(CurLine->Data, 1, strlen(CurLine->Data), Config);
		CurLine = CurLine->Next();
	}
	fclose(Config);
	
	bool Ret = FXFile::moveFiles(OGGDumpFile, ConfigFN, true);
	if(!Ret)	FXFile::removeFiles(OGGDumpFile);
	return Ret;
}

void ConfigFile::Clear()
{
	FileBuffer.Clear();
	Sect.Clear();
	SAFE_DELETE_ARRAY(ConfigFN);
}

ConfigFile::ConfigFile()
{
	ConfigFN = NULL;
}

ConfigFile::ConfigFile(const char* FN)
{
	ConfigFN = NULL;
	SetFN(FN);
	BufferFile();
}

ConfigFile::~ConfigFile()
{
	Clear();
}
// ----------
