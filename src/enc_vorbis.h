// Music Room Interface
// --------------------
// enc_vorbis.h - Built-in Vorbis encoding
// --------------------
// "©" Nmlgc, 2011

// Forward declarations
namespace FX
{
	class FXRealSlider;
	class FXLabel;
	class FXCheckButton;
}

struct OggVorbis_EncState;

struct Encoder_Vorbis : public Encoder
{
protected:
	// Settings
	// --------
	// Widget storage
	FXRealSlider* Q;
	FXLabel* EstBR;
	FXCheckButton* MS;

	void FmtReadConfig(ConfigParser* Sect);
	void FmtStop();

public:
	float	Quality;	// Vorbis quality setting
	bool	ChainStreamAssemble;	// Create chained bitstream output files where possible

	float	QualityToBitrate(const float& q);

	void	DlgCreate(FXVerticalFrame* Frame, FXDialogBox* Target, const FXuint& Msg);
	void	DlgPoll(FXDialogBox* Parent);
	bool	DlgApply(FXDialogBox* Parent);

	FXString Init(GameInfo* GI);

	bool Encode(const FXString& DestFN, const FXString& SrcFN, Extract_Vals& V);

	// Main extraction function.
	// Returns true if [TI] was correctly extracted to [EncFN].
	bool Extract(TrackInfo* TI, FXString& EncFN, GameInfo* GI, Extract_Vals& V);
};
