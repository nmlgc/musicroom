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
	char	Key[64];
	FXString Data;

	ConfigKey();
	ConfigKey(const char* _Key_);

	void	SetData(char* Parse);
	void	GetData(ushort DataType, void* Trg);
	void	SetInfo(const char* _Key_);
};

template class List<ConfigKey>;

class ConfigParser
{
	friend class ConfigFile;

protected:
	char	Caption[64];	// Section Name

	List<ConfigKey>	Keys;

public:
	void	Save();
	void	Load();

	ConfigKey*	FindKey(char* Name);

	void	Clear();

	ConfigParser();
	~ConfigParser();
};

class ConfigFile
{
	friend class ConfigParser;

protected:
	PList<char>	FileBuffer;
	List<ConfigParser>	Section;
	char		ConfigFN[256];

	bool	BufferFile();

	PListEntry<char>*	InsertLine(PListEntry<char>* PrevLine, char* NewLine);	// Inserts a new line into the buffer

	ConfigParser*	FindSection(const char* Name);
	ConfigParser*	CheckSection(char* Line);

public:
	void	Load();

	bool	GetValue(const char* Section, char* Key, ushort DataType, void* Value);

	void	Clear();

	ConfigFile();
	ConfigFile(const char* FN);
	~ConfigFile();
};

#endif /* LEGACY_CONFIG_H */
