// Music Room Interface
// --------------------
// widgets.h - Additional and modified GUI widgets
// --------------------
// "©" Nmlgc, 2011

// Default "OK", "Cancel" and "Apply" buttons in dialogs
// -------
class LCButtonFrame : public FXHorizontalFrame
{
	FXDECLARE(LCButtonFrame);

private:
	LCButtonFrame()	{}

public:
	FXButton*	OK;
	FXButton*	Apply;
	FXButton*	Cancel;

	LCButtonFrame(FXComposite* p, long ApplyID = -1);
};
// -------

// Modified Table
// --------------
class LCTable : public FXTable
{
public:
	LCTable(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);

	// Fit column widths to contents
	void fitColumnsToContents(FXint col,FXint nc=1);
};
// --------------

// Modified ListBox
// ----------------
class LCListBox : public FXListBox
{
public:
	LCListBox(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_SUNKEN|FRAME_THICK|LISTBOX_NORMAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);

	// Get popup pane
	FXPopup* getPane();
};
// ----------------

// Modified text box
// -----------------
class LCText : public FXText
{
public:
	LCText(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=3,FXint pr=3,FXint pt=2,FXint pb=2);

	// Sets this cool inverted color scheme
	void setColors();
};
// -----------------

// Wiki update confirmation
// ------------------------
class LCUpdateMessage : public FXDialogBox	// Thanks for having to copy the entire fucking class, Jeroen.
{
	FXDECLARE(LCUpdateMessage)

private:
	FXColor	CalcClr(const FXColor& Base, const FXColor& Text);
	void Construct(const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg, FXIcon* icn, FXuint whichbuttons);

protected:
	LCUpdateMessage()	{}
	LCUpdateMessage(const LCUpdateMessage&);
	LCUpdateMessage &operator=(const LCUpdateMessage&);

public:
	long onCmdClicked(FXObject*,FXSelector,void*);
	long onCmdCancel(FXObject*,FXSelector,void*);

	static const FXColor OldClr, NewClr;

	enum
	{
    ID_CLICKED_YES=FXDialogBox::ID_LAST,
    ID_CLICKED_NO,
    ID_CLICKED_OK,
    ID_CLICKED_CANCEL,
    ID_CLICKED_QUIT,
    ID_CLICKED_SAVE,
    ID_CLICKED_SKIP,
    ID_CLICKED_SKIPALL,
    ID_CLICKED_YESALL,
    ID_CLICKED_NOALL,
    ID_LAST
    };

	// Construct free floating message box with given caption, icon, and message texts
	LCUpdateMessage(FXApp* app,const FXString& caption,const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg,FXIcon* ic=NULL,FXuint opts=0,FXint x=0,FXint y=0);

	static FXuint question(FXApp* app, FXuint opts, const FXString& caption, const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg);
};
// ------------------------