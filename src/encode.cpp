// Music Room Interface
// --------------------
// encode.cpp - Encoder base class and settings dialog
// --------------------
// "©" Nmlgc, 2011

#include "musicroom.h"
#include <fx.h>
#include <bgmlib/config.h>
#include "widgets.h"
#include "enc_base.h"
#include "encode.h"

// Encoder base class
// ------------------
volatile bool Encoder::StopReq;
volatile bool Encoder::Active;

void Encoder::ReadConfig(ConfigParser* Sect)
{
	Sect->GetValue("ext", TYPE_STRING, &Ext);
	Sect->GetValue("lossless", TYPE_BOOL, &Lossless);
	FmtReadConfig(Sect);
}

void Encoder::Stop()
{
	StopReq = true;
	if(Active)	FmtStop();
	StopReq = false;
}
// ------------------

// Settings dialog
// ---------------
FXDEFMAP(EncSettingDlg) MMEncSettingDlg[] =
{
	FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, EncSettingDlg::onAccept),
	FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_CANCEL, EncSettingDlg::onCancel),
	// All kinds of messages we may possibly receive
	FXMAPFUNCS(SEL_COMMAND, EncSettingDlg::ES_UPDATE, EncSettingDlg::ES_UPDATE_END, EncSettingDlg::onUpdate),
	FXMAPFUNCS(SEL_CHANGED, EncSettingDlg::ES_UPDATE, EncSettingDlg::ES_UPDATE_END, EncSettingDlg::onUpdate),
};

FXIMPLEMENT(EncSettingDlg, FXDialogBox, MMEncSettingDlg, ARRAYNUMBER(MMEncSettingDlg));

EncSettingDlg::EncSettingDlg(FXWindow *Parent)
	: FXDialogBox(Parent, "Encoding settings", DECOR_TITLE | DECOR_BORDER | DECOR_CLOSE)
{
	FXuint EncID = ES_UPDATE;
	ListEntry<Encoder*>* CurEnc;
	Encoder* Enc;

	Tab = new FXTabBook(this, NULL, 0, TABBOOK_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0);
	
	ButtonFrame = new LCButtonFrame(this);

	CurEnc = Encoders.First();
	while(CurEnc)
	{
		Enc = CurEnc->Data;

		// FXTabBook always arranges pairs consisting of one FXTabItem and one layout manager
		new FXTabItem(Tab, Enc->Ext);

		Enc->DlgCreate(new FXVerticalFrame(Tab, LAYOUT_SIDE_TOP | FRAME_THICK | FRAME_RAISED), this, EncID);

		EncID++;
		CurEnc = CurEnc->Next();
	}
}

long EncSettingDlg::onAccept(FXObject* Sender, FXSelector Msg, void* ptr)
{
	ListEntry<Encoder*>* CurEnc = Encoders.First();
	while(CurEnc)
	{
		if(!CurEnc->Data->DlgApply(this))	return 1;
		CurEnc = CurEnc->Next();
	}
	return onCancel(Sender, Msg, ptr);
}

long EncSettingDlg::onCancel(FX::FXObject *, FX::FXSelector, void *)
{
	getApp()->stopModal(this, 1);
	return 1;
}

long EncSettingDlg::onUpdate(FXObject* Sender, FXSelector Msg, void* ptr)
{
	FXuint EncID;
	ListEntry<Encoder*>* Enc;

	EncID = (FXSELID(Msg) - ES_UPDATE);
	Enc = Encoders.Get(EncID);
	if(Enc)	Enc->Data->DlgPoll(this);
	return 1;
}
// ---------------

// Reads [size] bytes from [in] into [buffer]. Loops according to the info in [TI].
ulong pcm_read_bgm(FXFile& in, char* buffer, const ulong& size, TrackInfo* TI)
{
	long ReadSize;
	long Rem = size;
	ulong pos = in.position();

	ulong S, L, E;
	TI->GetPos(FMT_BYTE, true, &S, &L, &E);
	
	while(Rem > 0)
	{
		if((pos + Rem) >= E)	ReadSize = E - pos;
		else					ReadSize = Rem;
			
		ReadSize = in.readBlock(buffer + size - Rem, ReadSize);
		Rem -= ReadSize;
		pos += ReadSize;

		if(pos == E)
		{
			if(L != E)	pos = L;
			else		pos = S;
			in.position(pos);
		}
	}
	return pos;
}
