// Music Room Interface
// --------------------
// tag_id3v2.cpp - ID3v2 Tagging
// --------------------
// "©" Nmlgc, 2011

#include <bgmlib/platform.h>
#include <FXFile.h>
#include <bgmlib/infostruct.h>
#include <bgmlib/utils.h>
#include <bgmlib/bgmlib.h>
#include "tag_base.h"
#include "tag_id3v2.h"

// Synchsafe 32-bit integer class
// ------------------------------
ss32::ss32()													{ss = false; e = SysEnd; d = 0;}
ss32::ss32(const ss32& s)										{set(s.d, s.e, s.ss);}
ss32::ss32(const ulong &_d, const bool& _e, const bool& _ss)	{set(_d, _e, _ss);}

ulong& ss32::tosynchsafe()
{
	// Thanks, Wikipedia!
	ulong out = 0, mask = 0x7F;
 
	while (mask ^ 0x7FFFFFFF)
	{
		out = d & ~mask;
		out <<= 1;
		out |= d & mask;
		mask = ((mask + 1) << 8) - 1;
		d = out;
	}
	ss = true;
	return d;
}

ulong& ss32::unsynchsafe()
{
	ulong out = 0, mask = 0x7F000000;

	while(mask)
	{
		out >>= 1;
		out |= d & mask;
		mask >>= 8;
	}
	ss = false;
	return d = out;
}

void ss32::set(const ulong& data, const bool& endian, const bool& synchsafe)
{
	ss = synchsafe;
	return EndVal::set(data, endian);
}

ulong& ss32::swap()
{
	d = EndianSwap(d);
	e = !e;
	return d;
}

ulong& ss32::val(const bool& endian, const bool& synchsafe)
{
	if(ss != synchsafe)
	{
		if(ss)
		{
			EndVal::val(endian);
			unsynchsafe();
		}
		else
		{
			tosynchsafe();
			EndVal::val(endian);
		}
	}
	else	EndVal::val(endian);
	return d;
}
// ------------------------------

const FXString FNMap_ID3v2[MAP_END] = {"TXXX", "", "TPE1", "TCOM", "TIT2", "TALB", "COMM", "", "TRCK", "", "TPOS", "TCON", "TDRC"};

// ID3v2Header
// -----------
const char ID3v2Header::ID3Sig[3] = {'I', 'D', '3'};
const u8 ID3v2Header::SpecSize = 10;

void ID3v2Header::Read(FXFile& File)
{
	ulong t32;
	ushort t16;

	File.position(0);
	File.readBlock(&Magic, 3);
	File.readBlock(&t16, 2);
	File.readBlock(&Flags, 1);
	File.readBlock(&t32, 4);

	Version.set(t16, BE);
	TagSize.set(t32, BE, true);
}

bool ID3v2Header::Verify()
{
	u8* S;
	u8 c;
	// Step 1: ID3 magic
	if(strncmp(Magic, ID3Sig, 3))	return false;
	// Step 2: Version
	S = ((uchar*)&Version.val());
	for(c = 0; c < 2; c++)	if(S[c] == 0xFF) return false;
	// Step 3: Size synchsafe-ness
	if(TagSize.synchsafe())
	{
		S = ((uchar*)&TagSize.val());
		for(c = 0; c < 4; c++)	if(S[c] & 0x80)	return false;
	}

	return true;
}
// -----------

// ID3v2FrameHeader
// ----------------
const u8 ID3v2FrameHeader::SpecSize = 10;

bool ID3v2FrameHeader::Read(FXFile& File)
{
	ulong t32 = 0;

	File.readBlock(&ID, 4);
	if(!memcmp(&t32, ID, 4))
	{
		memset(this, 0, sizeof(ID3v2FrameHeader));
		return false;
	}
	File.readBlock(&t32, 4);
	File.readBlock(&Flags, 2);

	Size.set(t32, BE, true);
	return true;
}
// ----------------

// Tag writer class
// ----------------
char* ID3v2FrameHeader::Create(Field* Fi, const char* ID, ss32 S, ushort Fl, bool SetUTF8)
{
	if(!Fi || !ID)	return NULL;

	char* p;

	    Fi->Len = SpecSize + S(SysEnd, false);
	p = Fi->Tag = new char[Fi->Len];

	memcpy_advance(&p, ID, 4);	// Frame ID
	memcpy_advance(&p, &S(BE, true), 4);	// Size (synchsafe)
	memcpy_advance(&p, &Fl, 2);	// Flags
	if(SetUTF8)	{*p = 0x3;	p++;}	// UTF-8 encoding
	return p;
}

char* ID3v2FrameHeader::Create(Field* F, bool SetUTF8)
{
	return Create(F, ID, Size, Flags, SetUTF8);
}

uint MRTag_ID3v2::WriteNative(Field* F, const char* ID, const FXString* Data)
{
	if(!Data)	Data = F->Data;

	ss32 Size(Data->length() + 2);

	char* p = ID3v2FrameHeader::Create(F, ID, Size);

	memcpy(p, Data->text(), Size - 1);
	return F->Len;
}

uint MRTag_ID3v2::WriteCustom(Field* F, const FXString& ID, const FXString* Data)
{
	if(!Data)	Data = F->Data;

	FXint SL[2] = {ID.length() + 1, Data->length() + 1};
	ss32 Size(1 + SL[0] + SL[1]);
	
	char* p = ID3v2FrameHeader::Create(F, FNMap_ID3v2[MAP_BEGIN].text(), Size);

	memcpy_advance(&p, ID.text(), SL[0]);
	memcpy_advance(&p, Data->text(), SL[1]);
	return F->Len;
}

uint MRTag_ID3v2::FmtRenderField(Field *F, const FXString& CustomID)
{
	const FXchar* ID = NULL;
	FXString CT;
	Field* TT;

	// ID3v2 native tags
	// -----------------
	switch(F->Name)
	{
	case ARTIST:
	case COMPOSER:
	case TITLE:	
	case ALBUM:	
	case DISCNUMBER:	
	case GENRE:	
	case YEAR:			return WriteNative(F, FNMap_ID3v2[F->Name].text());
	case TOTALTRACKS:	return 0;	// Gets combined with TRACK
	}
	// -----------------

	// Custom tags
	// -----------
	switch(F->Name)
	{
	case CIRCLE:
	case ALBUM_ARTIST:	return WriteCustom(F, FNMap_Custom[F->Name]);

	case TRACK:
		ID = FNMap_ID3v2[F->Name].text();
		TT = Find(TOTALTRACKS);
		if(!TT)	return WriteNative(F, ID);

		CT = *F->Data + '/' + *TT->Data;
		return WriteNative(F, ID, &CT);

	case COMMENT:
		ss32 Size(3 + 1 + F->Data->length() + 2);

		char* p = ID3v2FrameHeader::Create(F, FNMap_ID3v2[F->Name].text(), Size);

		memcpy_advance(&p, BGMLib::LI[Lang].Code3.text(), 4);
		strncpy(p, F->Data->text(), F->Data->length() + 1);

		return F->Len;
	}
	// -----------

	if(!CustomID.empty())	return WriteCustom(F, CustomID);

	return 0;
}

char* MRTag_ID3v2::FmtGet(Field* F)
{
	// Search for the last null-terminated substring starting after the header.
	// Lazy solution, I know.
	uint s, t;
	for(s = t = ID3v2Header::SpecSize + 1; t < (F->Len - 1); t++)	{if(F->Tag[t] == '\0')	s = ++t;}
	return &F->Tag[s];
}

bool MRTag_ID3v2::ParseCustom(FXFile& In, ID3v2FrameHeader& FH)
{
	ulong FrameSize = FH.Size(SysEnd, false);
	FXint s = 1, t;
	FXString Buf, IDStr;
	FieldName FNID;

	Buf.length(FrameSize);
	In.readBlock(Buf.text(), FrameSize);
	// Get the first substring, which will be the field name
	t = Buf.find('\0', s);
	if(t == -1)	return false;

	IDStr = Buf.mid(s, t-s);
	FNID = MRTag::ParseCustom(IDStr);

	Field* F = Find(FNID);
	if(!F)
	{
		F = &TF.Add()->Data;
		F->Name = FNID;

		char* p = FH.Create(F, false);
		memcpy(p, Buf.text(), FrameSize);
		return true;
	}
	else	return false;
}

bool MRTag_ID3v2::ParseNative(FXFile& In, ID3v2FrameHeader& FH, const FieldName& ID)
{
	ulong FrameSize = FH.Size(SysEnd, false);
	Field* F = Find(ID);
	if(!F)
	{
		F = &TF.Add()->Data;
		F->Name = ID;

		char* p = FH.Create(F, false);
		return In.readBlock(p, FrameSize) != 0;
	}
	// We already have a field with this ID, skip...
	In.position(FrameSize, FXIO::Current);
	return false;
}

bool MRTag_ID3v2::FmtOpen(FXFile& In, FXint* FTP, FXint* FTS, FXint* SSP)
{
	In.position(0);
	H.Read(In);
	if(!H.Verify())
	{
		// No ID3v2 tag present
		return true;
	}
	*FTP = 0;
	*FTS = *SSP = H.TagSize(LE, false) + ID3v2Header::SpecSize;
	return true;
}

bool MRTag_ID3v2::FmtReadTags(FXFile& In)
{
	if(H.Version != 4)	return false;

	In.position(H.SpecSize);

	FXString ID;
	FXshort FNID;
	ID3v2FrameHeader FH;
	while(FH.Read(In))
	{
		ID.assign(FH.ID, 4);

		// Check custom tags first, because CUSTOM is included in the map match below
		if(ID == FNMap_ID3v2[CUSTOM])	ParseCustom(In, FH);
		else
		{
			FNID = FindMapMatch(ID, FNMap_ID3v2, MAP_END);
			if(FNID != -1)	ParseNative(In, FH, (FieldName)FNID);
			else
			{
				// Some strange frame we don't know
				Field* F = &TF.Add()->Data;
				memcpy(&F->Name, FH.ID, 4);

				char* p = FH.Create(F, false);
				return In.readBlock(p, FH.Size) != 0;
			}
		}
	}
	return true;
}

// Strip ID3v1, without mercy.
// We tend to overwrite those values with ID3v2 anyway, so nothing of value will be lost.
FXulong MRTag_ID3v2::StripID3v1(FXFile& In, bool NewFile)
{
	// But APE happens to have 'APETAGEX' as it's magic number, so we have to 
	// check for the rare case where we might accidentally an APE tag
	static const char ID3v1Sig[3] = {'T', 'A', 'G'};
	static const char APESig[3] = {'A', 'P', 'E'};
	static const FXint ID3v1Size = 128;

	char FileSig[6];
	FXint FS = In.size();

	In.position(-ID3v1Size - 3, FXIO::End);
	In.readBlock(FileSig, 6);
	if(!memcmp(FileSig + 3, ID3v1Sig, 3) && memcmp(FileSig, APESig, 3))
	{
		FS -= ID3v1Size;
		if(!NewFile)
		{
			In.position(-3, FXIO::Current);
			WriteByteBlock(In, ID3v1Size, 0);
		}
	}
	return FS;
}

mrerr MRTag_ID3v2::FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize)
{
	ulong c = 0;
	ListEntry<Field>*	CurTF;

	if(TagSize < 10)	return ERROR_GENERIC;

	// Subtract header
	if(*Rewrite == -1)
	{
		StripID3v1(Out, false);
	}
	
	Out.position(0);
	ushort Version(4);
	H.TagSize.set(TagSize - 10, LE, false);

	Out.writeBlock(ID3v2Header::ID3Sig, 3);
	Out.writeBlock(&Version, 2);
	Out.writeBlock(&c, 1);
	Out.writeBlock(&H.TagSize(BE, true), 4);

	CurTF = TF.First();
	while(CurTF)
	{
		Out.writeBlock(CurTF->Data.Tag, CurTF->Data.Len);
		c += CurTF->Data.Len;
		CurTF = CurTF->Next();
	}
	WriteByteBlock(Out, H.TagSize(LE, false) - c, 0);

	if(*Rewrite > -1)
	{
		*StreamSize = StripID3v1(In, true) - *Rewrite;
	}
	return SUCCESS;
}
// ----------------
