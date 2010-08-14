// Touhou Project BGM Extractor
// ----------------------------
// extract.h - Extracting and encoding functions
// ----------------------------
// "©" Nameless, 2010

extern const ushort WAV_HEADER_SIZE;

uint ExtractTrack(TrackInfo* TI);
void TagTrack(TrackInfo* TI, FXString& TagFN, FXString& Ext);
