// Music Room Interface
// --------------------
// enc_custom.h - External encoding
// --------------------
// "©" Nmlgc, 2011

#ifndef MUSICROOM_ENC_CUSTOM_H
#define MUSICROOM_ENC_CUSTOM_H

// Forward declarations
namespace FX
{
	class FXTextField;
}

struct Encoder_Custom : public Encoder
{
protected:
	// Widget storage
	FXTextField* Cmd[2];

	bool Ret;
	void FmtStop();	// Terminates the encoding process

	void FmtReadConfig(ConfigParser* Sect);	// Reads [encoder] -> <CmdLine[0]> and [options] -> <CmdLine[1]>

public:
	FXString	CmdLine[2];	// Encoding command line
	FXString	Help;		// Encoder help text (displayed in the settings dialog)

	// Settings
	void DlgCreate(FXVerticalFrame* Frame, FXDialogBox* Target, const FXuint& Msg);
	bool DlgApply(FXDialogBox* Parent);

	FXString Init(GameInfo* GI);	// Displays the encoding command line

	// Encodes [SrcFN] (PCM) to [DestFN] (target format).
	// Returns false if extraction has to be canceled, and true in _all_ other cases.
	bool Encode(const FXString& DestFN, const FXString& SrcFN, Extract_Vals& V);

	// Main extraction function.
	// Returns true if [TI] was correctly extracted to [EncFN].
	bool Extract(TrackInfo* TI, FXString& EncFN, GameInfo* GI, Extract_Vals& V);

	~Encoder_Custom();
};

#endif /* MUSICROOM_ENC_CUSTOM_H */