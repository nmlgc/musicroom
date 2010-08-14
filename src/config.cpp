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

	for(c = 0; c < MaxChars - 1; c++)
	{
		if(!fread(String + c, 1, 1, File))
		{
			if(c == 0)	return c;
			else
			{
				// Reached EOF, terminate string
				String[c - 1] = '\0';
				break;
			}
		};

		if(String[c] == 0x0A && String[c - 1] == 0x0D)
		{
			String[c - 1] = '\0';
			break;
		}
	}
	return c;
}

// Transforms a string to lowercase
bool LowerString(const char* Source, char* Dest)
{
	if(!Source || !Dest) return false;

	bool Lock = false;

	ushort Length = strlen(Source) + 1;
	for(ushort c = 0; c < Length; c++)
	{
		if(Source[c] == '"')	Lock = !Lock;

		// This function isn't used for any other purpose, so we can safely do this...
		if(Source[c] == '=')
		{
			strncpy(&Dest[c], &Source[c], Length - 1);
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
	Key[0] = '\0';
}

ConfigKey::ConfigKey(const char* _Key_)
{
	SetInfo(_Key_);
}

void ConfigKey::SetInfo(const char* _Key_)
{
	if(!_Key_ )	return;

	LowerString(_Key_, Key);

}

void ConfigKey::SetData(char* Parse)
{
	Data = Parse;
}

FXint BaseCheck(FXString& Str)
{
	FXint Base = 10;
	if(Str.left(2) == "0x")
	{
		Str.erase(0, 2);
		Base = 16;
	}
	return Base;
}

void ConfigKey::GetData(ushort DataType, void* Trg)
{
	FXint Base = 10;

	switch(DataType)
	{
	case TYPE_BOOL:	
		if(Data == "true")			*(bool*)Trg = true;
		else if(Data == "false")	*(bool*)Trg = false;
		else                    	*(bool*)Trg = atoi(Data.text()) != 0;
		break;

	case TYPE_SHORT:	*(short*)Trg = (short)Data.toInt(BaseCheck(Data));		break;
	case TYPE_USHORT:	*(ushort*)Trg = (ushort)Data.toUInt(BaseCheck(Data));	break;
	case TYPE_INT:		*(int*)Trg = Data.toInt(BaseCheck(Data));				break;
	case TYPE_UINT:		*(uint*)Trg = Data.toUInt(BaseCheck(Data));				break;
	case TYPE_LONG:		*(long*)Trg = Data.toLong(BaseCheck(Data));				break;
	case TYPE_ULONG:	*(ulong*)Trg = Data.toULong(BaseCheck(Data));			break;
	case TYPE_FLOAT:	*(float*)Trg = (float)Data.toFloat();					break;
	case TYPE_UCHAR:	*(uchar*)Trg = (uchar)Data.toUInt(BaseCheck(Data));		break;
	case TYPE_STRING:
		// Remove leading (and trailing) quotation marks
		Data.unescape('"', '"');
		strcpy((char*)Trg, Data.text());			break;
	}
}
// ---------

// ConfigParser
// ------------
ConfigParser::ConfigParser()
{
	Caption[0] = '\0';
}

ConfigKey* ConfigParser::FindKey(char* Name)
{
	ListEntry<ConfigKey>* CurKey = Keys.First();

	while(CurKey)
	{
		if(!strcmp(CurKey->Data.Key, Name))	return &CurKey->Data;
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
bool ConfigFile::BufferFile()
{
	char CurLine[2048], NewLine[2048];
	int Len;

	strcpy(NewLine, "# Legacy Configuration File\n");
	FileBuffer.Add(NewLine, strlen(NewLine) + 1);

	FILE* Config = fopen(ConfigFN, "rb");
	if(!Config) return false;//Log.Error("Config file not found!");

	// Skip byte order mark, if necessary
	fread(&CurLine, 3, 1, Config);
	if(CurLine[0] != (char)(0xEF) && CurLine[1] != (char)(0xBB) && CurLine[2] != (char)(0xBF))	fseek(Config, 0, SEEK_SET);

	while(Len = ReadLineFromFile(CurLine, 2048 * sizeof(char), Config))
	{
		if(CurLine[0] != '#' && CurLine[0] != '\0')
		{
			LowerString(CurLine, NewLine);
			FileBuffer.Add(NewLine, Len);
		}
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

void ConfigFile::Clear()
{
	FileBuffer.Clear();
}

ConfigParser* ConfigFile::FindSection(const char* Name)
{
	ListEntry<ConfigParser>* CurSect = Section.First();

	while(CurSect)
	{
		if(!strcmp(CurSect->Data.Caption, Name))	return &CurSect->Data;
		CurSect = CurSect->Next();
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
		NewSection = &(Section.Add()->Data);
		strcpy(NewSection->Caption, Temp);
	}
	SAFE_DELETE_ARRAY(Temp);
	return NewSection;
}

void ConfigFile::Load()
{
	char*         CurLine = NULL;
	ConfigParser* CurSection = NULL;

	char	Key[32];
	char	Equals[32];
	char	Value[2048];

	ConfigKey*	TrgKey;

	PList<char> Split;
	PListEntry<char>* CurSS;

	PListEntry<char>* BufferLine = FileBuffer.First();
	while(BufferLine = BufferLine->Next())
	{
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
		if(CurSS == Split.Last())	strcpy(Value, CurSS->Data);
		else
		{
			Value[0] = '\0';
			while(CurSS)
			{
				strcat(Value, CurSS->Data);
				if(CurSS != Split.Last())	strcat(Value, " ");
				CurSS = CurSS->Next();
			}
		}

		if(Equals[0] == '=')
		{
			TrgKey = CurSection->FindKey(Key);
			
			if(!TrgKey)
			{
				TrgKey = &(CurSection->Keys.Add()->Data);
				strcpy(TrgKey->Key, Key);
			}

			TrgKey->SetData(Value);
		}
	}
}

bool ConfigFile::GetValue(const char* SectStr, char* KeyStr, ushort DataType, void* Value)
{
	ConfigParser* Section = FindSection(SectStr);
	if(!Section)	return false;

	ConfigKey* Key = Section->FindKey(KeyStr);
	if(!Key)	return false;

	Key->GetData(DataType, Value);
	return true;
}

ConfigFile::ConfigFile()
{

}

ConfigFile::ConfigFile(const char* FN)
{
	strcpy(ConfigFN, FN);
	BufferFile();
}

ConfigFile::~ConfigFile()
{
	Clear();
}
// ----------
