// Legacy Engine
// -------------
// config.h - Parsing structures for simple cross-platform reading and writing of configuration files
// -------------
// "©" Nameless, 2008

#pragma once

#ifndef LEGACY_CONFIG_H
#define LEGACY_CONFIG_H

#define TYPE_BOOL	0x1
#define TYPE_SHORT	0x2
#define TYPE_USHORT	0x3
#define TYPE_INT	0x4
#define TYPE_UINT	0x5
#define TYPE_LONG	0x6
#define TYPE_ULONG	0x7
#define TYPE_FLOAT	0x8
#define TYPE_UCHAR	0x9
#define TYPE_STRING	0xA

struct ConfigKey
{
	friend class ConfigParser;
	friend class ConfigFile;

protected:
	FXString Key;
	void*	Data;
	ushort	DataType;

	FXString Line;	// Storage for the value read from the config file

	bool	SaveData(FXString* SaveLine);

public:
	ConfigKey();
	ConfigKey(FXString& Key, void* Data, ushort DataType);

	void	Link(void* Data, ushort DataType);	// Links this key to the [DataType] variable [Data]. On ConfigFile::Save, the current value of [Data] gets automatically written
	void	SetInfo(FXString& Key, void* Data, ushort DataType);

	bool	GetData(void* Data, ushort DataType, FXString* Line = NULL);	// Parses the value of [Line] and writes it to the [DataType] variable [Data]
};

template class List<ConfigKey>;

#define ADD_KEY(Type)	ConfigKey* AddKey(FXString& Key, Type* Data)

class ConfigParser
{
	friend class ConfigFile;

protected:
	FXString	Caption;	// Section Name

	List<ConfigKey>	Keys;

public:
	void	Save();
	void	Load();

	void	SetCaption(FXString& Caption);

	ADD_KEY(bool);
	ADD_KEY(short);
	ADD_KEY(ushort);
	ADD_KEY(int);
	ADD_KEY(uint);
	ADD_KEY(long);
	ADD_KEY(ulong);
	ADD_KEY(float);
	ADD_KEY(FXString);

	ConfigKey*	FindKey(char* Name);

	void	Clear();

	ConfigParser();
	~ConfigParser();
};

struct LineLink
{
	int Line;
	ConfigKey* Key;
};

class ConfigFile
{
	friend class ConfigParser;

protected:
	PList<char>	FileBuffer;
	List<ConfigParser> Sect;	// Automatically created sections
	List<LineLink> Link;	// Sequentially stored line-key links
	char*		ConfigFN;

	void	SetFN(const char* FN);
	bool	BufferFile();
	bool	WriteBuffer();

	PListEntry<char>*	InsertLine(PListEntry<char>* PrevLine, char* NewLine);	// Inserts a new line into the buffer

	ConfigParser*	FindSection(const char* Name);
	ConfigParser*	CheckSection(char* Line);

public:
	void	Load();
	bool	Load(const char* FN);

	char*	GetFN()	{return ConfigFN;}

	ConfigKey* FindKey(const char* Section, char* Key);

	bool	 GetValue(const char* Section, char* Key, ushort DataType, void* Value);	// Stores the parsed value of [Key] in [Section] in the [DataType] variable [Value]. Should only be used if [Value] is local or you're not going to save anyway
	bool	LinkValue(const char* Section, char* Key, ushort DataType, void* Value);	// Links [Key] in [Section] to the [DataType] variable [Value]. On ConfigFile::Save, the current value of [Value] gets automatically written

	bool	Save();

	void	Clear();

	ConfigFile();
	ConfigFile(const char* FN);
	~ConfigFile();
};

#endif /* LEGACY_CONFIG_H */
