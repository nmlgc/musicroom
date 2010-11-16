// Touhou Project BGM Extractor
// ----------------------------
// wikiupdate.cpp - Touhou Wiki Track Info Updating
// ----------------------------
// "©" Nameless, 2010

#include "thbgmext.h"
#include <curl.h>
#include <../lib/icons.h>

// Wiki stuff to remove
const FXString Tmpl[2] = {"{{", "}}"};
const FXString Link[2] = {"[[", "]]"};
const FXString HTML[2] = {"<", ">"};

// Removes all occurrences of a given substring
// No idea why there isn't a function like this already.
// --------------------------------------------
FXString& remove_sub(FXString& str, const FXchar* org, FXint olen)
{
  if(0<olen){
	  register FXint pos = 0;
    while(pos<=str.length()-olen){
      if(compare(str.text()+pos,org,olen)==0)
	  {
		  str.erase(pos,olen);
        continue;
		}
      pos++;
      }
    }
  return str;
  }

FXString& remove_sub(FXString& str, const FXchar& org)
{
  return remove_sub(str, &org, 1);
}

FXString& remove_sub(FXString& str, const FXString& org)
{
	return remove_sub(str, org.text(), org.length());
}
// --------------------------------------------

// Nicely colored custom message box
// ---------------------------------
class LCUpdateMessage : public FXDialogBox	// Thanks for having to copy the entire fucking class, Jeroen.
{
	FXDECLARE(LCUpdateMessage)

private:
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

const FXColor LCUpdateMessage::OldClr = FXRGB(255, 0, 0);
const FXColor LCUpdateMessage::NewClr = FXRGB(0, 255, 0);

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

void LCUpdateMessage::Construct(const FXString& TopMsg, const FXString& OldMsg, const FXString& NewMsg, FXIcon* icn, FXuint whichbuttons)
{
  FXButton *initial;
  FXVerticalFrame* content=new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame* info=new FXHorizontalFrame(content,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10);
  new FXLabel(info,FXString::null,icn,ICON_BEFORE_TEXT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame* msgs=new FXVerticalFrame(info,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  FXLabel* Top = new FXLabel(msgs,TopMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);

  FXMatrix* Mat = new FXMatrix(msgs, 2, MATRIX_BY_COLUMNS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	new FXLabel(Mat, "Old:", NULL, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_RIGHT);
	FXLabel* Old = new FXLabel(Mat,OldMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);	Old->setTextColor(OldClr);
	new FXLabel(Mat, "New:", NULL, JUSTIFY_RIGHT | LAYOUT_FILL_X | LAYOUT_RIGHT);
	FXLabel* New = new FXLabel(Mat,NewMsg,NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X);	New->setTextColor(NewClr);

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
// ---------------------------------

size_t write_data(char* File, size_t size, size_t nmemb, FXString* Page)
{
	Page->append(File, size * nmemb);

	return size * nmemb;
}

void RemoveWikiLinks(FXString& Str, const FXString* Del = Link)
{
	FXint s = 0, t = 0, c = 0, len;

	while((s = Str.find(Del[0], s)) != -1)
	{
		t = Str.find('|', s);
		c = Str.find(Del[1], s);

		if((uint)t < (uint)c)	len = t - s + 1;
		else                 	len = 2;

		Str.erase(s, len);
		c -= len;
		Str.erase(c, 2);
	}
}

void RemoveTemplate(FXString& Str, const FXString& Name, const FXString* Del = Tmpl)
{
	FXint s = 0;
	FXString Sub;
	if(Name.left(2) == Del[0])	Sub = Name;
	else						Sub = Del[0] + Name;

	while((s = Str.find(Sub, s)) != -1)
	{
		Str.erase(s, Sub.length());
		s = Str.find(Del[1], s);
		Str.erase(s, 2);
	}
}

void RemoveHTMLPairs(FXString& Str, const FXString* Del = HTML)
{
	FXint s = 0, t = 0, c = 0, len;

	while((s = Str.find(Del[0], s)) != -1)
	{
		while(Ascii::isSpace(Str[s - 1]))	s--;

		t = Str.find(Del[1], s);	// Closing bracket of the start tag 
		c = Str.find(Del[1], t + 1);// Closing bracket of the end tag

		len = MAX(t,c) - s + 1;
		
		Str.erase(s, len);
	}
}

FXString GetTemplateElm(FXString& WD, const FXString& Tag, FXint s = 0)
{
	FXint t;

	if((s = WD.find(Tag, s))                == -1)	return "";
	if((s = WD.find('=', s + Tag.length())) == -1)	return "";
	if((t = WD.find('|', s))                == -1)	return "";
	return WD.mid(s + 1, t - s - 1).trim();
}

FXuint AskForUpdate(TrackInfo* TI, const FXString& Type, FXString* Old, FXString* New, FXuint* Ret, bool ShowStat = true)
{
	if(New->empty() || *Ret == MBOX_CLICKED_NOALL) return *Ret;

	// Remove line breaks for comparison
	FXString o = *Old, n = *New;
	o.simplify();
	n.simplify();
	remove_sub(o, ' ');
	remove_sub(n, ' ');
	if(o == n)	return *Ret;

	FXString Msg;
	if(*Ret != MBOX_CLICKED_YESALL && !Old->empty())
	{
		Msg.format("Update %s on Track #%d?\n", Type, TI->Number);
		*Ret = LCUpdateMessage::question(MW->getApp(), MBOX_YES_YESALL_NO_NOALL_CANCEL, PrgName, Msg, *Old, *New);

		if(*Ret != MBOX_CLICKED_YES && *Ret != MBOX_CLICKED_YESALL)	return *Ret;
	}
	if(ShowStat)	Msg.format("Update on Track #%d: %s -> %s\n", TI->Number, *Old, *New);
	else			Msg.format("Updated %s on Track #%d\n", Type, TI->Number);
	MW->PrintStat(Msg);
	*Old = *New;
	return *Ret;
}

const FXString Wiki_Name[2] = {"title", "titleEN"};
const FXString Wiki_Comment[2] = {"comment", "translation"};

FXuint UpdateTrack(GameInfo* GI, FXint ID, FXString WD, FXuint* MsgRet)
{
	IntString Name, Comment;
	TrackInfo* Cur;

	remove_sub(WD, "}}");

	Name[LANG_JP] = GetTemplateElm(WD, Wiki_Name[LANG_JP]);	RemoveHTMLPairs(Name[LANG_JP]);
	Name[LANG_EN] = GetTemplateElm(WD, Wiki_Name[LANG_EN]);	RemoveHTMLPairs(Name[LANG_EN]);
	Comment[LANG_JP] = GetTemplateElm(WD, Wiki_Comment[LANG_JP]);
	Comment[LANG_EN] = GetTemplateElm(WD, Wiki_Comment[LANG_EN]);

	ListEntry<TrackInfo>* CurTrack = GI->Track.First();
	for(ushort Temp = 0; (Temp < GI->TrackCount) && CurTrack; Temp++)	// No spoilers for trial versions! :-)
	{
		Cur = &(CurTrack->Data);

		if( (Cur->Name[LANG_JP] == Name[LANG_JP]) || (Cur->Name[LANG_EN] == Name[LANG_EN])
#ifdef _VERY_PARANOID_
			(Cur->Comment[LANG_JP] == Comment[LANG_JP]) ||	// Um... Do we really want to get THAT far?
			(Cur->Comment[LANG_EN] == Comment[LANG_EN])
#endif
			)
		{
			// OK, found the right track
			if(AskForUpdate(Cur, "Japanese Name", &Cur->Name[LANG_JP], &Name[LANG_JP], MsgRet) == MBOX_CLICKED_CANCEL)	return *MsgRet;
			if(AskForUpdate(Cur, "English Name", &Cur->Name[LANG_EN], &Name[LANG_EN], MsgRet) == MBOX_CLICKED_CANCEL)	return *MsgRet;
			if(AskForUpdate(Cur, "Japanese Comment", &Cur->Comment[LANG_JP], &Comment[LANG_JP], MsgRet, false) == MBOX_CLICKED_CANCEL)	return *MsgRet;
			if(AskForUpdate(Cur, "English Comment", &Cur->Comment[LANG_EN], &Comment[LANG_EN], MsgRet, false) == MBOX_CLICKED_CANCEL)	return *MsgRet;
			
			CurTrack = NULL;
		}
		else	CurTrack = CurTrack->Next();
	}
	return *MsgRet;
}

bool Update(GameInfo* GI)
{
	if(GI->WikiPage.empty())	return false;

	bool Ret = true;
	FXuint MsgRet;

	FXString URL, Page, Str;
	FXint s = 0, t, ID = 1;
	CURL* C = NULL;
	FXColor Base = MW->getApp()->getBaseColor();

	ulong RevID;

	Str.format("Getting current track info from Touhou Wiki (%s)...", GI->WikiPage);
	MW->PrintStat(Str);

	GI->WikiPage.substitute(' ', '_');

	URL.format(WikiURL.text(), GI->WikiPage);

	curl_global_init(CURL_GLOBAL_ALL);

	C = curl_easy_init();

	curl_easy_setopt(C, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(C, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(C, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(C, CURLOPT_WRITEDATA, &Page);
	curl_easy_setopt(C, CURLOPT_URL, URL);

	CURLcode CRet = curl_easy_perform(C);
	if(CRet != CURLE_OK)
	{
		switch(CRet)
		{
		case CURLE_COULDNT_RESOLVE_HOST:	Str = "Couldn't resolve URL, no internet connection!";
		break;
		case CURLE_OPERATION_TIMEDOUT:		Str = "Timeout reached!";
		break;
		default:	Str = "Couldn't connect to Wiki page!";
		}
		Str.prepend("\nERROR: ");
		Str.append('\n');
		MW->PrintStat(Str);
		Ret = false;
		goto Cleanup;
	}

	// Get revision id
	// ---------------
	if((s = Page.find("<revision>")) != 1)
	{
		s = Page.find("<id>", s) + 4;
		t = Page.find("</id>", s);
		RevID = Page.mid(s, t - s).toULong();
	}

	if(RevID == GI->WikiRev)
	{
		MW->PrintStat("\nTrack info is already up-to-date.\n");
		Ret = false;
		goto Cleanup;
	}
	// ---------------

	MW->PrintStat("done, evaluating...\n");

	// Prepare buffer
	// --------------
	if((s = Page.find("<text xml:space=\"preserve\">", s)) == -1)
	{
		MW->PrintStat("Wiki data is corrupted, update not possible!");
		Ret = false;
		goto Cleanup;
	}

	Page = Page.right(Page.length() - s);
	s = 0;
		
	remove_sub(Page, LineBreak[1]);
	RemoveTemplate(Page, "lang|en|");
	RemoveTemplate(Page, "lang|jp|");
	
	RemoveWikiLinks(Page);

	// Empty space in some comments
	const char Empty[2] = {LineBreak[1], ' '};
	FXchar Moonspace[4] = {' ', (char)0xE3, (char)0x80, (char)0x80};	// STUPID "IDEOGRAPHIC WHITE SPACE"

	Page.substitute(Empty, 2, &LineBreak[1], 1, true);

									Page.substitute(&Moonspace[0], 4, &Moonspace[0], 1, true);
	Moonspace[0] = LineBreak[1];	Page.substitute(&Moonspace[0], 4, &Moonspace[0], 1, true);
	Moonspace[0] = '=';         	Page.substitute(&Moonspace[0], 4, &Moonspace[0], 1, true);
	
	Page.substitute("  ", " ");
	remove_sub(Page, "  ");
	// HTML
	Page.substitute("&gt;", ">");
	Page.substitute("&lt;", "<");

	// I'm not in the mood to write functions anymore
	while((s = Page.find("<br", s)) != -1)
	{
		t = Page.find(">", s);

		Page.replace(s, t-s+1, &LineBreak[1]);
		s++;
	}

	s = 0;

	while((s = Page.find("{{ref", s)) != -1)
	{
		t = Page.find("}}", s);

		Page.erase(s, t-s+1);
		s++;
	}
	Page.substitute("&quot;", "\"");
	// --------------

	s = 0;

	// Make our fancy colors actually readable
	// MW->getApp()->setBaseColor(FXRGB(0, 0, 0));

	while((s = Page.find("{{MusicRoom", s)) != -1)
	{
		s = Page.find('|', s);
		t = Page.find("}}", s);

		Page[t] = '|'; t++;

		UpdateTrack(GI, ID, Page.mid(s, t - s), &MsgRet);
		
		if(MsgRet == MBOX_CLICKED_CANCEL)
		{
			MW->PrintStat("Update canceled.\n");
			Page.clear();
			Ret = false;
			goto Cleanup;
		}
		else if(MsgRet == MBOX_CLICKED_NOALL)	s = Page.length() + 1;
		ID++;
	}
	MW->PrintStat("Update completed.\n");
	GI->WikiRev = RevID;

Cleanup:
	curl_easy_cleanup(C);
	curl_global_cleanup();

	MW->getApp()->setBaseColor(Base);
	
	return Ret;
}
