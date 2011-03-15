// Music Room Interface
// --------------------
// tag_id3v2.h - ID3v2 Tagging
// --------------------
// "©" Nmlgc, 2011

// Synchsafe 32-bit integer class
// ------------------------------
struct ss32 : public EndVal<ulong>
{
protected:
	bool	ss;

	ulong& tosynchsafe();
	ulong& unsynchsafe();
	
public:
	ss32();
	ss32(const ss32& s);
	ss32(const ulong& data, const bool& endian = SysEnd, const bool& synchsafe = false);

	bool& synchsafe()	{return ss;}
	ulong& val()	{return d;}

	ulong& swap();
	void set(const ulong& data, const bool& endian = SysEnd, const bool& synchsafe = false);

	ulong& val(const bool& endian, const bool& synchsafe = false);
	ulong& operator ()	(const bool& endian, const bool& synchsafe = false)	{return val(endian, synchsafe);}
};
// ------------------------------

// Mapping array (FieldName -> native ID3 Frame ID)
extern const FXString FNMap_ID3v2[MAP_END];

// ID3v2Header
// -----------
struct ID3v2Header
{
	static const char ID3Sig[3];	// "ID3"
	static const u8	SpecSize;		// Size of this structure (10 bytes)

	char Magic[3];	// [3 bytes] ID3v2 identifier ("ID3")
	u16 Version;	// [2 bytes] version (0x0400)
	u8  Flags;		// [1  byte] flags
	ss32 TagSize;	// [4 bytes] Size of the whole tag (minus this header)

	void Read(FXFile& File);
	bool Verify();
};
// -----------

// ID3v2FrameHeader
// ----------------
struct ID3v2FrameHeader
{
	friend class MRTag_ID3v2;

public:
	static const u8	SpecSize;		// Size of this structure (10 bytes)

	char ID[4];	// [4 bytes] Frame ID
	ss32 Size;	// [4 bytes] Frame Size
	ushort Flags;//[2 bytes] Flags

	static char* Create(Field* F, const char* ID, ss32 TagSize, ushort Flags = 0, bool SetUTF8 = true);	// Returns pointer to [F->Tag] after the header
	char* Create(Field* F, bool SetUTF8 = true);	// Returns pointer to [F->Tag] after the header

	bool Read(FXFile& File);	// returns false if no tag is present at the current [File] read position. Can indicate if reading has reached the padding area
};
// ----------------

// Tag writer class
// ----------------
class MRTag_ID3v2 : public MRTag
{
protected:
	uint WriteNative(Field* F, const char* ID, const FXString* Data = NULL);
	uint WriteCustom(Field* F, const FXString& ID, const FXString* Data = NULL);

	uint FmtRenderField(Field* F, const FXString& CustomID);
	char* FmtGet(Field* F);

	// Parse the following custom ID3 frame [FH] with [ID] in <File>
	bool ParseCustom(FXFile& In, ID3v2FrameHeader& FH); 
	// Parse the following custom ID3 frame [FH] with [ID] in <File>
	bool ParseNative(FXFile& In, ID3v2FrameHeader& FH, const FieldName& ID);

	// Strip ID3v1, without mercy
	FXulong StripID3v1(FXFile& In, bool NewFile);	

	bool FmtReadTags(FXFile& In);

	bool FmtOpen(FXFile& In, FXint* TagStart, FXint* TagSize, FXint* StreamStart);
	mrerr FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize);

public:
	ID3v2Header	H;
};
// ----------------
