// Music Room Interface
// --------------------
// enc_base.h - Encoder base class
// --------------------
// "©" Nmlgc, 2011

#ifndef MUSICROOM_ENC_BASE_H
#define MUSICROOM_ENC_BASE_H

// Forward declarations
class ConfigParser;
struct Extract_Vals;

namespace FX
{
	class FXVerticalFrame;
	class FXDialogBox;
}

#define MAX_ENCODERS	16

// Encoder
// -------
struct Encoder
{
protected:
	virtual void FmtReadConfig(ConfigParser* Sect)	{}

	// Optional handling if encoding gets stopped by the user
	virtual void FmtStop()	{}

public:
	static volatile bool Active;
	static volatile bool StopReq;

	FXString	Ext;	// File extension
	bool		Lossless;	// Lossless encoder?

	// Setting dialog methods
	// - DlgCreate: Construct individual widgets on top of [Frame]. Initialize them with the class values.
	//              If the subclass should receive poll messages, link them to [Target] and [Msg].
	//(- DlgPoll: Some widget was changed, do optional stuff)
	// - DlgApply: Copy values 
	virtual void DlgCreate(FXVerticalFrame* Frame, FXDialogBox* Target, const FXuint& Msg)	{}
	virtual void DlgPoll(FXDialogBox* Parent)	{}
	// Copies the values back to the encoder class. May return false if there was some error and the dialog should not be closed yet.
	virtual bool DlgApply(FXDialogBox* Parent)	{return true;}

	// Optional handling before the extraction starts. May return a custom status string to display.
	virtual FXString Init(GameInfo* GI)	{return "";}

	// Reads [Ext] and [Lossless], then calls FmtReadConfig for further processing
	void ReadConfig(ConfigParser* Sect);

	// Encodes [SrcFN] (PCM) to [DestFN] (target format).
	// Returns false if extraction has to be canceled, and true in _all_ other cases.
	virtual bool Encode(const FXString& DestFN, const FXString& SrcFN, Extract_Vals& V)	{return false;}

	// Main extraction function.
	// Returns true if [TI] was correctly extracted to [EncFN].
	virtual bool Extract(TrackInfo* TI, FXString& EncFN, GameInfo* GI, Extract_Vals& V)	{return false;}

	// Default implementation dumps [TI] to a PCM file, then calls Encode()
	bool Extract_Default(TrackInfo* TI, FXString& EncFN, GameInfo* GI, Extract_Vals& V);

	// Requests encoding cancellation. Sets <StopReq> and calls FmtStop
	void Stop();	
	virtual ~Encoder()	{}
};
// -------

// Reads [size] bytes from [in] into [buffer]. Loops according to the info in [TI].
ulong pcm_read_bgm(FXFile& in, char* buffer, const ulong& size, TrackInfo* TI);

#endif /* MUSICROOM_ENC_BASE_H */
