// Music Room Interface
// --------------------
// tag_base.h - Basic definitions for the tagging engine
// --------------------
// "©" Nmlgc, 2011

// FieldName
// ---------
// Maximum number of multi-language fields
#define I18N_MAX 0x7

enum FieldName
{
	CUSTOM = 0x0,
	MAP_BEGIN = 0x0,
	// Multi-language fields
	CIRCLE = 0x1,
	ARTIST,
	COMPOSER,
	TITLE,
	ALBUM,
	COMMENT,
	// Language-independent fields
	ALBUM_ARTIST = I18N_MAX,
	TRACK,
	TOTALTRACKS,
	DISCNUMBER,
	GENRE,
	YEAR,
	MAP_END,
	// i18n
	I18N_BASE = 0x20,
	I18N_END = I18N_BASE + (LANG_COUNT * I18N_MAX)
};

FieldName operator + (const FieldName& a, const FieldName& b);
FieldName operator - (const FieldName& a, const FieldName& b);
FieldName operator % (const FieldName& a, const FieldName& b);

// Mapping array (FieldName -> custom frame name)
extern const FXString FNMap_Custom[MAP_END];

// Returns the index of [In] inside the string array [Map]
FXshort FindMapMatch(FXString& In, const FXString* Map, const FXshort& Limit);
// ---------

// Tagging field (only used internally)
// -------------
struct Field
{
	FieldName	Name;	// Field ID
	FXString*	Data;	// Pointer to the string to write in this field
	char*	Tag;	// Tag data in the requested format
	uint	Len;	// Length of <Tag>

	Field();
	~Field();
};
// -------------

// Tag writer base class (subclassed for the different tagging formats)
// ---------------------
class MRTag
{
private:
	FXFile	File;

protected:
	static FXuint Drive;	// No need to save a string when we can just compare a value! (yeah, this is my first time)
	static ushort ClusterSize;	// Cluster size of the tag file's drive to perfectly align tag padding

	FXString FN;	// Name of the file to tag
	bool	RO;	// Is the file read-only?
	
	FXint	FTP;	// Tag start position in the source file
	FXint	FTS;	// Size of the tag (w/ padding) already present in the file
	FXint	SSP;	// Start of the audio data in the source file
	
	List<Field>	TF;	// Fields to write

	// Search for a field name matching [ID]. Mainly takes care of i18n
	FieldName ParseCustom(FXString ID);

	// Format-specific
	// ---------------
	// Fills out [F]->Tag and [F]->Len
	virtual uint FmtRenderField(Field* F, const FXString& CustomID) = 0;

	// Optional value to add to the result of the RenderAll() function
	virtual uint FmtTagHeaderSize()	{return 0;}

	// File opening. Should return start positions of the tag and stream data inside the source file.
	virtual bool FmtOpen(FXFile& In, FXint* TagStart, FXint* TagSize, FXint* StreamStart) = 0;

	// Returns the value of [F]. May optionally return the ID of the tag in the subclassed format.
	virtual char* FmtGet(Field* F) {return "";}

	virtual bool FmtReadTags(FXFile& In) = 0;

	// Tag save processing.
	// This function should always write to [Out], [In] is read-only.
	// If [Rewrite] is not -1, the caller will copy [StreamSize] bytes from [In] starting at [Rewrite].
	virtual mrerr FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize) = 0;
	// ---------------

	uint RenderField(Field* F);
	// Renders all fields and returns their total size
	uint RenderAll();

public:
	static bool	SetDrive(const FXString& Path);	// Sets the cluster size of [Path]'s drive

	inline bool ReadOnly()	{return RO;}

	Field* Find(const FieldName& Name);
	Field* Add(const FieldName& Name, FXString* Data);
	char* Get(const FieldName& Name);	// Returns the value of the [Name] field

	mrerr Open(const FXString& FN);
	bool ReadTags();	// Updates <TF> with the tags already present in the loaded file. If this isn't called, all present tags will be removed on writing!
	mrerr Save();	// Save all fields to the opened file

	void Clear();	// Clears elements of the base class

	MRTag();
	MRTag(const FXString& FN)	{Open(FN);}

	// C++ skill lesson #?: 
	// You create a instance of a subclass, but only save a pointer to the base class.
	// And if you call delete on that one, the subclass destructor is never called.
	// Unless your destructor is virtual.
	virtual ~MRTag();
};
// ---------------------
