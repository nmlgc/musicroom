// Music Room Interface
// --------------------
// enc_vorbis.h - Built-in Vorbis encoding
// --------------------
// "©" Nmlgc, 2011

#ifndef MUSICROOM_ENC_VORBIS_H
#define MUSICROOM_ENC_VORBIS_H

// Forward declarations
namespace FX
{
	class FXCheckButton;
}

struct OggVorbis_EncState;
class LCVorbisQuality;

struct Encoder_Vorbis : public Encoder
{
protected:
	// Settings
	// --------
	// Widget storage
	LCVorbisQuality* VQ;
	FXCheckButton* MS;

	void FmtReadConfig(ConfigParser* Sect);
	void FmtStop();

public:
	float	Quality;	// Vorbis quality setting
	bool	ChainStreamAssemble;	// Create chained bitstream output files where possible

	void	DlgCreate(FXVerticalFrame* Frame, FXDialogBox* Target, const FXuint& Msg);
	bool	DlgApply(FXDialogBox* Parent);

	FXString Init(GameInfo* GI);

	bool Encode(const FXString& DestFN, const FXString& SrcFN, Extract_Vals& V);

	// Main extraction function.
	// Returns true if [TI] was correctly extracted to [EncFN].
	bool Extract(TrackInfo* TI, FXString& EncFN, GameInfo* GI, Extract_Vals& V);
};

#endif /* MUSICROOM_ENC_VORBIS_H */