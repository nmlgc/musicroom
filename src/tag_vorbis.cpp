// Music Room Interface
// --------------------
// tag_vorbis.cpp - Vorbis comment Tagging
// --------------------
// "©" Nmlgc, 2011

#include <bgmlib/platform.h>
#include <FXFile.h>
#include <bgmlib/infostruct.h>
#include <bgmlib/utils.h>
#include <bgmlib/libvorbis.h>
#include "tag_base.h"
#include "tag_vorbis.h"

// Vorbis comment base class
// -------------------------
MRTag_VC::MRTag_VC()
{
	vorbis_comment_init(&vc);
}

uint MRTag_VC::FmtRenderField(Field* F, const FXString& CustomID)
{
	FXString Str;
	FXString Tag;
	
	if(CustomID.empty() && F->Name < MAP_END)	Tag = FNMap_Custom[F->Name];
	else										Tag = CustomID;
	int c, d;
	
	for(c = 0; c < vc.comments; c++)
	{
		for(d = 0; vc.user_comments[c][d] != '='; d++);

		Str = FXString(vc.user_comments[c], d);

		if(Tag == Str)	break;
	}

	if(c < vc.comments)
	{
		// Replace tag
		Str.format("%s=%s", Tag, *F->Data);

		FXint DataLen = Str.length();
		if(DataLen > vc.comment_lengths[c])
		{
			vc.user_comments[c] = (char*)_ogg_realloc(vc.user_comments[c], DataLen + 1);
		}
		
		strcpy(vc.user_comments[c], Str.text());
		vc.comment_lengths[c] = DataLen;
	}
	else
	{
		vorbis_comment_add_tag(&vc, Tag.text(), F->Data->text());
		c = vc.comments - 1;
	}
	
	F->Tag = vc.user_comments[c];
	return F->Len = (vc.comment_lengths[c] + 4);
}

char* MRTag_VC::FmtGet(Field* F)
{
	int d;
	for(d = 0; F->Tag[d] != '='; d++);
	d++;

	return &F->Tag[d];
}

void MRTag_VC::ParseVC()
{
	Field* F;
	FXString ID;
	FieldName FNID;
	int d;

	for(int c = 0; c < vc.comments; c++)
	{
		for(d = 0; vc.user_comments[c][d] != '='; d++);

		ID = FXString(vc.user_comments[c], d);

		FNID = ParseCustom(ID);

		F = Find(FNID);
		if(!F)
		{
			F = &TF.Add()->Data;
			F->Name = FNID;
			F->Len = (vc.comment_lengths[c] + 4);
			F->Tag = vc.user_comments[c];
		}
	}
}

ulong MRTag_VC::WriteVC(FXFile& Out)
{
	ulong w = 0;
	w += Out.writeBlock(&vc.comments, 4);

	for(int c = 0; c < vc.comments; c++)
	{
		w += Out.writeBlock(&vc.comment_lengths[c], 4);
		w += Out.writeBlock(vc.user_comments[c], vc.comment_lengths[c]);
	}
	return w;
}

MRTag_VC::~MRTag_VC()
{
	// Disassociate tag pointers (data is owned by vc)
	ListEntry<Field>* CurTF = TF.First();
	while(CurTF)
	{
		CurTF->Data.Tag = NULL;
		CurTF = CurTF->Next();
	}
}
// -------------------------

// Ogg Vorbis
// ----------
MRTag_Ogg::MRTag_Ogg()
{
	ogg_sync_init(&oy);
	vorbis_info_init(&vi);
	memset(&os, 0, sizeof(ogg_stream_state));
}

bool MRTag_Ogg::FmtOpen(FXFile& In, FXint* FTP, FXint* FTS, FXint* SSP)
{
	ogg_page         og; // one Ogg bitstream page.  Vorbis packets are inside
	ogg_packet       op; // one raw packet of data for decode
   	FXuint bytes;
	
	In.position(0);

	ogg_sync_init(&oy);
	bytes = ogg_update_sync(In, &oy);

	// Get the first page.
    if(ogg_sync_pageout(&oy,&og)!=1)
	{
      // error case.  Must not be Vorbis data
      return false;
    }
  
    // Get the serial number and set up the rest of decode.
    // serialno first; use it to set up a logical stream
    ogg_stream_init(&os,ogg_page_serialno(&og));
    
    // extract the initial header from the first page and verify
	// that the Ogg bitstream is in fact Vorbis data
    
    // I handle the initial header first instead of just having the code
    // read all three Vorbis headers at once because reading the initial
    // header is an easy way to identify a Vorbis bitstream and it's
    // useful to see that functionality seperated out.
    vorbis_info_init(&vi);
    // error; stream version mismatch perhaps
    if(ogg_stream_pagein(&os,&og)<0)	return false;  
	// no page? must not be vorbis
    if(ogg_stream_packetout(&os,&op)!=1)return false;
    // error case; not a vorbis header
    if(vorbis_synthesis_headerin(&vi,&vc,&op)<0)return false;

	int i=0;
	while(i<2)
	{
		while(i<2)
		{
			int result=ogg_sync_pageout(&oy,&og);
			if(result==0)break; // Need more data
			/* Don't complain about missing or corrupt data yet. We'll
			   catch it at the packet output phase */
			if(result==1)
			{
				// we can ignore any errors here as they'll also become apparent at packetout
				ogg_stream_pagein(&os,&og); 
				while(i<2)
				{
					result=ogg_stream_packetout(&os,&op);
					if(result==0)break;
					/* Uh oh; data at some point was corrupted or missing!
					   We can't tolerate that in a header.  Die. */
					if(result<0)	return false;
	            
					result=vorbis_synthesis_headerin(&vi,&vc,&op);
					i++;
					if(result<0)	break;
				}
			}
		}
		bytes = ogg_update_sync(In, &oy);
		// End of file before finding all Vorbis headers!
		if(bytes==0 && i<2)	return false;
	}

	// Because of chained bitstreams, we're better off with always rewriting the file
	*FTS = 0;
	// for(int c = 0; c < vc.comments; c++)	*FTS += vc.comment_lengths[c] + 4;
	return true;
}

bool MRTag_Ogg::FmtReadTags(FXFile& In)
{
	ParseVC();
	return true;
}

static void copy_packet(ogg_packet* dst, ogg_packet* src)
{
	dst->packet = (unsigned char*)realloc(dst->packet, src->bytes);
	memcpy(dst->packet, src->packet, src->bytes);
	dst->b_o_s = src->b_o_s;
	dst->e_o_s = src->e_o_s;
	dst->bytes = src->bytes;
	dst->granulepos = src->granulepos;
	dst->packetno = src->packetno;
}

mrerr MRTag_Ogg::FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize)
{
	ogg_stream_state	stream_out;
	ogg_packet header;
	ogg_packet header_code;
	ogg_page og;
	ogg_packet op;

	memset(&header, 0, sizeof(ogg_packet));
	memset(&header_code, 0, sizeof(ogg_packet));
	
	// No, just rewriting the page if enough PAD is present in the file is not working for whatever reason...
	ogg_stream_init(&stream_out, os.serialno);
	vorbis_write_headers(Out, &stream_out, &vi, &vc);

	// Rewrite file by copying the Ogg pages, instead of doing a raw copy of the old file
	// Be sure to copy back every single bitstream!
	while(1)
	{
		int c = 0;
		ogg_packetcopy(Out, &stream_out, In, &os, &oy);

		if(ogg_update_sync(In, &oy) == 0)	break;

		ogg_stream_clear(&os);
		ogg_stream_reset_serialno(&stream_out, ++stream_out.serialno);

		// We should always read the Vorbis format and codebook headers from the stream!
		// If the stream format happens to change after all, just writing the first headers
		// everywhere will quickly produce a broken file.
		while(c < 3)
		{
			int result=ogg_sync_pageout(&oy,&og);
			if(result > 0)
			{
				// Init [stream_in] if necessary
				if(os.body_data == NULL)
				{
					ogg_stream_init(&os, ogg_page_serialno(&og));
				}
				ogg_stream_pagein(&os,&og); // can safely ignore errors at this point
				while(c < 3)
				{
					result=ogg_stream_packetout(&os,&op);
					if(result<=0)break; // need more data

					switch(c)
					{
					case 0:	copy_packet(&header, &op);	break;
					case 2:	copy_packet(&header_code, &op);	break;
					}
					c++;
				}
			}
			else if(c < 3)	ogg_update_sync(In, &oy);
		}
		vorbis_write_headers(Out, &stream_out, &vi, &header, &vc, &header_code);
	}
	*Rewrite = -1;
	ogg_stream_clear(&stream_out);

	SAFE_FREE(header.packet);
	SAFE_FREE(header_code.packet);
	
	return SUCCESS;
}

MRTag_Ogg::~MRTag_Ogg()
{
	ogg_stream_clear(&os);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
	ogg_sync_clear(&oy);
}
// ----------

// FLAC
// ====

// FLAC headers
// ------------
const char FLAC_Header::FlacSig[4] = {'f', 'L', 'a', 'C'};
const u8 FLAC_Header::SpecSize = 42;
// ------------

bool FLAC_Header::Read(FX::FXFile &File)
{
	File.readBlock(Magic, 4);

	if(strncmp(Magic, FlacSig, 4))	return false;

	FLAC_BlockHeader StreamInfo;
	File.position(StreamInfo.Read(File), FXIO::Current);
	return true;
}

ulong FLAC_BlockHeader::Read(FX::FXFile &File)
{
	File.readBlock(&BT, 1);

	Last = (BT & 0x80) != 0;
	BT = (FLAC_BlockType)(BT & 0x7F);
	if(BT > BT_LAST)	return 0;

	Size = 0;
	File.readBlock(&Size, 3);
	Size <<= 8;
	Size = EndianSwap(Size);

	return Size;
}

bool MRTag_Flac::FmtOpen(FXFile& In, FXint* FTP, FXint* FTS, FXint* SSP)
{
	FLAC_Header	FH;
	FLAC_BlockHeader FBH;

	VCClear();

	char LastBT = 0;	// needed to see if the padding is in the right place
	*FTS = 0;
	*SSP = 0;
	FLAC_BlockType PosType = STREAMINFO;	// Which block has VCPos?
		
	uint BlockSize;

	if(!FH.Read(In))	return false;
	
	FBH.Last = 0;
	*FTP = In.position();
	while(!FBH.Last)
	{
		BlockSize = FBH.Read(In);
		if(!BlockSize)	return false;
		if(FBH.BT == VORBIS_COMMENT)
		{
			if(PosType != VORBIS_COMMENT)
			{
				*FTP = In.position();
				PosType = FBH.BT;
			}
			TagIsLast = FBH.Last;
			*FTS += BlockSize;

			*SSP = In.position() + BlockSize;
		}
		else if(FBH.BT == PADDING)
		{
			if(LastBT == VORBIS_COMMENT)
			{
				*FTS += BlockSize + 4;
				TagIsLast = FBH.Last;
				*SSP = In.position() + BlockSize;
			}
			else if(PosType != VORBIS_COMMENT)
			{
				*FTS += BlockSize + 4;
				*FTP = In.position();
				PosType = FBH.BT;
			}
		}

		In.position(BlockSize, FXIO::Current);
		LastBT = FBH.BT;
	}

	if(!*SSP)	*SSP = In.position();

	return true;
}

bool MRTag_Flac::FmtReadTags(FXFile& In)
{
	uint VendorSize;
	// I'm using my own implementation here because libvorbis resizes the structures with every new tag
	In.position(FTP);

	In.readBlock(&VendorSize, 4);	// vendor string length
	vc.vendor = new char[VendorSize + 1];
	In.readBlock(vc.vendor, VendorSize);
	vc.vendor[VendorSize] = '\0';
	
	In.readBlock(&vc.comments, 4);
	if(vc.comments < 1)
	{
		vc.comments = 0;
		return false;
	}

	vc.comment_lengths = new int[vc.comments];
	vc.user_comments = new char*[vc.comments];
	for(int c = 0; c < vc.comments; c++)
	{
		if(In.readBlock(&vc.comment_lengths[c], 4) != 4)	return false;
		vc.user_comments[c] = new char[vc.comment_lengths[c] + 1];
		In.readBlock(vc.user_comments[c], vc.comment_lengths[c]);
		vc.user_comments[c][vc.comment_lengths[c]] = '\0';
	}

	ParseVC();
	return true;
}

uint MRTag_Flac::FmtTagHeaderSize()
{
	uint Ret = 4;	// vendor string length
	if(vc.vendor)	Ret += strlen(vc.vendor);	// vendor string
	return Ret += 4;	// comment count
}

void MRTag_Flac::VCClear()
{
	vorbis_comment_clear(&vc);
}

static ulong ThreeByteSwap(ulong In)
{
	ulong Ret = EndianSwap(In);
	return (Ret >>= 8);
}

mrerr MRTag_Flac::FmtSave(FXFile& Out, FXint TagSize, FXFile& In, FXint* Rewrite, FXuint* StreamSize)
{
	int c = 0, Size, Rest;
	const int Null = 0;
	char BT = 0x4;
	if(*Rewrite == -1)
	{
		Out.position(FTP);
	}
	else
	{
		// Copy the source file from the beginning to the comment block
		char* Src = new char[FTP];
		In.position(0);
		In.readBlock(Src, FTP);
		Out.writeBlock(Src, FTP);
		SAFE_DELETE_ARRAY(Src);
	}

	Size = strlen(vc.vendor);
	c += Out.writeBlock(&Size, 4);
	if(vc.vendor)	c += Out.writeBlock(vc.vendor, Size);

	c += WriteVC(Out);

	Rest = (TagSize - c);

	// Rest is not big enough for even the FLAC block header...
	// Let's fill the few bytes with 0...
	if(BETWEEN(Rest, 0, 4))
	{
		Out.writeBlock(&Null, Rest);
		c += Rest;
	}
	
	// Write the total size in the block header
	Out.position(FTP - 4);
	if(c == TagSize)	BT |= 0x80;
	Out.writeBlock(&BT, 1);

	Size = ThreeByteSwap(c);
	Out.writeBlock(&Size, 3);
	Out.position(c, FXIO::Current);
	
	if(Rest >= 4)
	{
		Out.flush();
		// Write a FLAC padding block
		char BT = 0x1;
		if(TagIsLast)	BT |= 0x80;
		Out.writeBlock(&BT, 1);
		Rest -= 4;
		c = ThreeByteSwap(Rest);
		Out.writeBlock(&c, 3);
		WriteByteBlock(Out, Rest, 0);
	}
	return SUCCESS;
}

MRTag_Flac::~MRTag_Flac()
{
	VCClear();
}
// ====
