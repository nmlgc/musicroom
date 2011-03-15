// Music Room Interface
// --------------------
// tag_vorbis.h - Vorbis comment Tagging
// --------------------
// "©" Nmlgc, 2011

// Vorbis comment base class
// -------------------------
class MRTag_VC : public MRTag
{
protected:
	uint FmtRenderField(Field* F, const FXString& CustomID);

	char* FmtGet(Field* F);

	void ParseVC();	// Parses a filled <vc> structure
	ulong WriteVC(FXFile& Out);	// Writes a filled <vc> structure to [Out]. Returns the total number of bytes written.

public:
	vorbis_comment vc;

	MRTag_VC();
	virtual ~MRTag_VC();
};
// -------------------------

// Ogg Vorbis
// ----------
class MRTag_Ogg : public MRTag_VC
{
protected:
	ogg_stream_state os; // take physical pages, weld into a logical stream of packets
	ogg_sync_state   oy; // sync and verify incoming physical bitstream
	vorbis_info vi;
	bool FmtOpen(FXFile& In, FXint* TagStart, FXint* TagSize, FXint* StreamStart);

	mrerr FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize);

public:
	bool FmtReadTags(FXFile& In);

	MRTag_Ogg();
	~MRTag_Ogg();
};
// ----------

// FLAC
// ----

// FLAC headers
// ------------
struct FLAC_Header
{
	static const char FlacSig[4];	// "fLaC"
	static const u8 SpecSize;		// Size of this structure and the following STREAMINFO block (42 bytes)

	char Magic[4];	// [4 bytes] FLAC identifier ("fLaC")
	// And that's it, we don't need more here

	bool Read(FXFile& File);
};

enum FLAC_BlockType : char
{
	STREAMINFO = 0,
	PADDING,
	APPLICATION,
	SEEKTABLE,
	VORBIS_COMMENT,
	CUESHEET,
	PICTURE,
	BT_LAST
};

struct FLAC_BlockHeader
{
	bool	Last;		// [1   bit] Last-metadata-block flag: '1' if this block is the last metadata block before the audio blocks, '0' otherwise.
	FLAC_BlockType	BT;	// [7  bits] Block Type.
	ulong	Size;		// [3 bytes] Size of following metadata

	ulong Read(FXFile& File);	// Returns <Size> as int
};
// ------------

class MRTag_Flac : public MRTag_VC
{
protected:
	bool	TagIsLast;	// If true, writes the last-block-bit to the padding block added on saving

	bool FmtOpen(FXFile& In, FXint* FTP, FXint* FTS, FXint* SSP);
	bool FmtReadTags(FXFile& In);
	uint FmtTagHeaderSize();
	mrerr FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize);

	void VCClear();	// Clears the vorbis_comment structure

public:
	~MRTag_Flac();
};
// ----
