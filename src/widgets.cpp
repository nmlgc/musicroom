// Music Room Interface
// --------------------
// widgets.cpp - Additional and modified GUI widgets
// --------------------
// "©" Nmlgc, 2011

#include <bgmlib/platform.h>
#include <FXHash.h>
#include <FXStream.h>
#include <FXSize.h>
#include <FXPoint.h>
#include <FXRectangle.h>
#include <FXRegistry.h>
#include <FXEvent.h>
#include <FXWindow.h>
#include <FXPath.h>
#include <FXMessageBox.h>
#include <FXThread.h>
#include <FXColors.h>
#include <FXIcon.h>
#include <FXGIFIcon.h>
#include <FXSeparator.h>
#include <FXLabel.h>
#include <FXButton.h>
#include <FXHorizontalFrame.h>
#include <FXVerticalFrame.h>
#include <FXMessageBox.h>
#include <FXMatrix.h>
#include <FXApp.h>
#include <FXDC.h>
#include <FXObjectList.h>
#include <FXHeader.h>
#include <FXTable.h>
#include <FXList.h>
#include <FXListBox.h>
#include <FXText.h>

#include <../lib/icons.h>

#define FXINVCLR(rgb)	FXRGB(0xff - FXREDVAL(rgb), 0xff - FXGREENVAL(rgb), 0xff - FXBLUEVAL(rgb))

#include "widgets.h"

// Default "OK", "Cancel" and "Apply" buttons in dialogs
// -------
// "Message Map"
FXDEFMAP(LCButtonFrame) MMButtonFrame[] =
{
#ifdef _MSC_VER
0
#endif
};

FXIMPLEMENT(LCButtonFrame, FXHorizontalFrame, MMButtonFrame, ARRAYNUMBER(MMButtonFrame));

LCButtonFrame::LCButtonFrame(FXComposite* p, long ApplyID)
	: FXHorizontalFrame(p, LAYOUT_LEFT | LAYOUT_BOTTOM | LAYOUT_SIDE_BOTTOM | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0)
{
	OK = new FXButton(this, "  OK  ", NULL, p, FXDialogBox::ID_ACCEPT, BUTTON_NORMAL | LAYOUT_FILL_Y | LAYOUT_RIGHT);
	Cancel = new FXButton(this, "  Cancel  ", NULL, p, FXDialogBox::ID_CANCEL, BUTTON_NORMAL | LAYOUT_FILL_Y | LAYOUT_LEFT);

	if(ApplyID != -1)	Apply = new FXButton(this, "  Apply  ", NULL, p, ApplyID, BUTTON_NORMAL | LAYOUT_FILL_Y | LAYOUT_RIGHT);
}
// -------

// Modified Table
// --------------
LCTable::LCTable(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb)
	: FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb)
{
}

void LCTable::fitColumnsToContents(FX::FXint col, FX::FXint nc)
{
	register FXint c;

	FXHeader* Cap = getColumnHeader();
	FXHeaderItem* HI;
	FXint HISize;

	for(c=col; c<col+nc; c++)
	{
		HISize = 0;
		HI = Cap->getItem(c);
		if(HI)	HISize = HI->getWidth(Cap);

		setColumnWidth(c, MAX(getMinColumnWidth(c), HISize) + 5);
	}
}
// ---------------------

// Modified ListBox
// ----------------
LCListBox::LCListBox(FXComposite *p, FXObject *tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb)
	: FXListBox(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb)
{
	field->setLayoutHints(LAYOUT_FILL);
	field->setJustify(JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y);
	recalc();
	repaint();
}

FXPopup* LCListBox::getPane()	{return pane;}
// ----------------

// Modified text box
// -----------------
LCText::LCText(FXComposite *p, FXObject *tgt, FXSelector sel, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb)
	: FXText(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb)
{
}

void LCText::setColors()
{
	FXColor TmpClr = getBackColor();
	setBackColor(getTextColor());
	setTextColor(TmpClr);
	setCursorColor(TmpClr);
	setBarColor(TmpClr);
	setActiveBackColor(FXColors::RoyalBlue);
}
// -----------------

// Wiki update confirmation
// ------------------------
const FXColor LCUpdateMessage::OldClr = FXColors::Red;
const FXColor LCUpdateMessage::NewClr = FXColors::Green;

// Padding for message box buttons
#define HORZ_PAD 30
#define VERT_PAD 2

#define MBOX_BUTTON_MASK   (MBOX_OK|MBOX_OK_CANCEL|MBOX_YES_NO|MBOX_YES_NO_CANCEL|MBOX_QUIT_CANCEL|MBOX_QUIT_SAVE_CANCEL|MBOX_SAVE_CANCEL_DONTSAVE)

// Map
FXDEFMAP(LCUpdateMessage) MMLCUpdateMessage[]=
{
	FXMAPFUNC(SEL_COMMAND,LCUpdateMessage::ID_CANCEL,LCUpdateMessage::onCmdCancel),
	FXMAPFUNCS(SEL_COMMAND,LCUpdateMessage::ID_CLICKED_YES,LCUpdateMessage::ID_CLICKED_NOALL,LCUpdateMessage::onCmdClicked),
};

// Object implementation
FXIMPLEMENT(LCUpdateMessage,FXDialogBox,MMLCUpdateMessage,ARRAYNUMBER(MMLCUpdateMessage))

// Construct free floating message box with given caption, icon, and message texts
LCUpdateMessage::LCUpdateMessage(FXApp* a,const FXString& caption,const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg, FXIcon* icn, FXuint opts,FXint x,FXint y)
	: FXDialogBox(a,caption,opts|DECOR_TITLE|DECOR_BORDER,x,y,0,0, 0,0,0,0, 4,4)
{
	Construct(TopMsg, OldMsg, NewMsg, icn, opts&MBOX_BUTTON_MASK);
}

FXColor LCUpdateMessage::CalcClr(const FXColor& Base, const FXColor& Text)
{
	short bc[3] = {FXREDVAL(Base), FXGREENVAL(Base), FXBLUEVAL(Base)};
	return FXRGB( MIN(bc[0] - (short)FXREDVAL(Text), 0), MIN(bc[1] - (short)FXGREENVAL(Text), 0), MIN(bc[2] - (short)FXBLUEVAL(Text), 0));
}

void LCUpdateMessage::Construct(const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg, FXIcon* icn, FXuint whichbuttons)
{
	FXColor Base = getApp()->getBaseColor();
	FXButton *initial;
	FXVerticalFrame* content=new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXHorizontalFrame* info=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10);
	new FXLabel(info,FXString::null,icn,ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
	FXVerticalFrame* msgs=new FXVerticalFrame(info,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	FXLabel* Top = new FXLabel(msgs,TopMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);

  FXMatrix* Mat = new FXMatrix(msgs, 2, MATRIX_BY_COLUMNS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	new FXLabel(Mat, "Old:", NULL, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_RIGHT);
	FXLabel* Old = new FXLabel(Mat,OldMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);

	Old->setHiliteColor(makeShadowColor(Base));	Old->setShadowColor(OldClr);
	Old->disable();
	
	new FXLabel(Mat, "New:", NULL, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_RIGHT);
	FXLabel* New = new FXLabel(Mat,NewMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);

	New->setHiliteColor(makeShadowColor(Base));	New->setShadowColor(NewClr);
	New->disable();

  new FXHorizontalSeparator(content,SEPARATOR_GROOVE|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);
  FXHorizontalFrame* buttons=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,10,10,5,5);
  if(whichbuttons==MBOX_YES_YESALL_NO_NOALL_CANCEL)
  {
	  initial=new FXButton(buttons,tr("&Yes"),NULL,this,ID_CLICKED_YES,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
	  new FXButton(buttons,tr("Y&es to All"),NULL,this,ID_CLICKED_YESALL,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,tr("&No"),NULL,this,ID_CLICKED_NO,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
	new FXButton(buttons,tr("Ign&ore All"),NULL,this,ID_CLICKED_NOALL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    new FXButton(buttons,tr("&Cancel"),NULL,this,ID_CLICKED_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_CENTER_X,0,0,0,0,HORZ_PAD,HORZ_PAD,VERT_PAD,VERT_PAD);
    initial->setFocus();
	}
}

// Close dialog with a cancel
long LCUpdateMessage::onCmdClicked(FXObject*,FXSelector sel,void*){
  getApp()->stopModal(this,MBOX_CLICKED_YES+(FXSELID(sel)-ID_CLICKED_YES));
  hide();
  return 1;
  }


// Close dialog with a cancel
long LCUpdateMessage::onCmdCancel(FXObject* sender,FXSelector,void* ptr){
  return LCUpdateMessage::onCmdClicked(sender,FXSEL(SEL_COMMAND,ID_CLICKED_CANCEL),ptr);
  }

// Show a modal question dialog, in free floating window
FXuint LCUpdateMessage::question(FXApp* app,FXuint opts, const FXString& caption,const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg)
{
  FXGIFIcon icon(app,questionicon);
  LCUpdateMessage box(app,caption,TopMsg, OldMsg, NewMsg,&icon,opts|DECOR_TITLE|DECOR_BORDER);
  return box.execute(PLACEMENT_SCREEN);
}
// ------------------------