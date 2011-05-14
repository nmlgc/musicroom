// Music Room Interface
// --------------------
// encode.h - Encoding settings dialog
// --------------------
// "©" Nmlgc, 2011

#ifndef MUSICROOM_ENCODE_H
#define MUSICROOM_ENCODE_H

class EncSettingDlg : public FXDialogBox
{
	FXDECLARE(EncSettingDlg);

private:
	EncSettingDlg()	{}

protected:
	FXTabBook*		Tab;

	LCButtonFrame*	ButtonFrame;

public:
	EncSettingDlg(FXWindow* Parent);

	enum
	{
		ES_UPDATE = FXDialogBox::ID_LAST,	// Calls DlgPoll() on the corresponding encoder class
		ES_UPDATE_END = ES_UPDATE + MAX_ENCODERS,

		ID_LAST
	};

	MSG_FUNC(onAccept);
	MSG_FUNC(onCancel);
	MSG_FUNC(onUpdate);
};

#endif /* MUSICROOM_ENCODE_H */
