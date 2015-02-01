// (c) MJavad
// Fix for rtl languages (for now supports Persian and Arabic with LTR base)
#include <base/system.h>
#include "rtl_support.h"

void CRTLFix::FixString(char *pDst, const char *pSrc, int DstSize, bool FullRTL, int *pCursor, int *pFixedLen)
{
	int Cursor = -1;
	int Size = 0;
	int Char;
	char Buf[4];
	CChar Str[MAX_RTL_FIX_SRT_LEN];

	// Fill the buffer with pSrc and set the Cursor
	const char *pTmpSrc = pSrc;
	while((Char = str_utf8_decode(&pTmpSrc)) && Size < MAX_RTL_FIX_SRT_LEN-1) // max size is MAX_RTL_FIX_SRT_LEN-1
		if(Char != -1)
		{
			if(Cursor == -1 && pCursor && *pCursor < pTmpSrc-pSrc)
				Cursor = Size;
			Str[Size].Type = GetLinkType(Char);
			Str[Size++].Char = Char;
		}
	if(Cursor == -1 && pCursor && *pCursor < pTmpSrc-pSrc)
		Cursor = Size;

	// Unusual things
	if(Size == 0 || DstSize < 2)
	{
		if(DstSize > 0)
			*pDst = 0;
		return;
	}
	if(Size == 1)
	{
		int s = str_utf8_encode(Buf, Str[0].Char);
		if(FullRTL)
			if(Cursor == 0)
				*pCursor = s;
			else
				*pCursor = 0;
		else
			if(Cursor == 0)
				*pCursor = 0;
			else
				*pCursor = s;
		if(s > DstSize)
			s = DstSize-1;
		mem_copy(pDst, Buf, s);
		pDst[s] = 0;
		return;
	}

	// Process the buffer
	// Step 1: Skip NO_BREAK chars and set PrevType and NextType
	Str[0].PrevType = LTR;
	Str[Size].Type = LTR;
	int *TypeToFill = 0;
	int *TypeToFill2 = 0;
	int *CharToFill = 0;
	int PrevType;
	for(int i = 0; i < Size; i++)
	{
		int t = Str[i].Type;
		int c = Str[i].Char;

		if(c == ARABIC_TATWEEL)
		{
			Str[i+1].PrevType = DUAL;
			Str[i].Type = NONE;
		}

		if(t == NO_BREAK)
			continue;

		if(CharToFill && Cursor != i)
		{
			for(int j = 0; j < ARABIC_ALEF_CHARS_N; j++)
				if(c == ARABIC_ALEF_CHARS[j])
					{
						*CharToFill = ARABIC_LAMALEF_CHARS[j];
						*TypeToFill2 = BEFORE;
						PrevType = BEFORE;
						Str[i].Char = 0;
						break;
					}
			CharToFill = 0;
		}

		if(!Str[i].Char)
			continue;

		if(TypeToFill)
		{
			*TypeToFill = t;
			TypeToFill = 0;
			Str[i].PrevType = PrevType;
		}


		int nt = Str[i+1].Type;

		if(nt == NO_BREAK || c == ARABIC_LAM)
		{
			if(c == ARABIC_LAM)
			{
				CharToFill = &Str[i].Char;
				TypeToFill2 = &Str[i].Type;
			}
			TypeToFill = &Str[i].NextType;
			PrevType = t;
		}
		else
		{
			Str[i].NextType = nt;
			Str[i+1].PrevType = t; // i+1 exists because max size is MAX_RTL_FIX_SRT_LEN-1
		}
	}
	if(TypeToFill)
		*TypeToFill = 0;

	// Step 2: Link chars (the HARD part!) and set the Cursor (ALL PARTS have copyright and too much time spent on them)
	int c;
	int t;
	int pt;
	int nt;
	int s;
	int RIndex = 0;
	int NIndex = 0;
	int LIndex = 0;
	int RCursor = -1;
	int NCursor = -1;
	int LCursor = -1;
	const int MaxSize = MAX_RTL_FIX_SRT_LEN*8; // Make sure we never reach the end (4 = max utf8 int char size, 2 = for 2 sides buffers)
	char RBuf[MaxSize];	// RTL buffer
	char NBuf[MaxSize];	// NO_DIR buffer (2 sides)
	char LBuf[MaxSize];	// LTR buffer
	char *pDstMax = pDst+DstSize-1;
	char *pOldDst = pDst;
	for(int i = 0; i < Size; i++)
	{
		t = Str[i].Type;
		c = Str[i].Char;
		if(!c)
			continue;
		if(FullRTL)		// i need a rest :)
		{
			switch(t)
			{
			case LTR:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						LCursor = LIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(LBuf+LIndex, NBuf, NIndex);
					LIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					LCursor = LIndex;
				mem_copy(LBuf+LIndex, Buf, s);
				LIndex += s;

				break;
			case NUMBER:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						LCursor = LIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(LBuf+LIndex, NBuf, NIndex);
					LIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					LCursor = LIndex;
				mem_copy(LBuf+LIndex, Buf, s);
				LIndex += s;

				break;
			case NO_DIR:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}
				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case NONE:

				c = GetLinked(c);
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}
				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case BEFORE:

				c = GetLinked(c);
				if(Str[i].PrevType == DUAL)
					c += FINAL;
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}
				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case DUAL:

				c = GetLinked(c);
				pt = Str[i].PrevType;
				nt = Str[i].NextType;
				if(pt == DUAL)
					if(nt == BEFORE || nt == DUAL)
						c += MEDIAL;
					else
						c += FINAL;
				else
					if(nt == BEFORE || nt == DUAL)
						c += INITIAL;
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}
				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case NO_BREAK:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}
				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			}
		}
		else
		{
			switch(t)
			{
			case LTR:

				if(RIndex)
				{
					if(RCursor != -1)
					{
						*pCursor = pDst-pOldDst+RIndex-RCursor;
						RCursor = -1;
						Cursor = -1;
					}
					mem_copy(pDst, RBuf+MaxSize-RIndex, RIndex);
					pDst += RIndex;
					RIndex = 0;
				}
				if(NIndex)
				{
					if(NCursor != -1)
					{
						*pCursor = pDst-pOldDst+NCursor;
						NCursor = -1;
						Cursor = -1;
					}
					mem_copy(pDst, NBuf, NIndex);
					pDst += NIndex;
					NIndex = 0;
				}
				if(LIndex)
				{
					if(LCursor != -1)
					{
						*pCursor = pDst-pOldDst+NCursor;
						LCursor = -1;
						Cursor = -1;
					}
					mem_copy(pDst, LBuf, LIndex);
					pDst += LIndex;
					LIndex = 0;
				}

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst)
					break;
				if(Cursor == i)
				{
					*pCursor = pDst-pOldDst;
					Cursor = -1;
				}
				mem_copy(pDst, Buf, s);
				pDst += s;

				break;
			case NUMBER:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(RIndex)
				{
					if(Cursor == i)
						LCursor = LIndex;
					mem_copy(LBuf+LIndex, Buf, s);
					LIndex += s;
				}
				else
				{
					if(Cursor == i)
					{
						*pCursor = pDst-pOldDst;
						Cursor = -1;
					}
					mem_copy(pDst, Buf, s);
					pDst += s;
				}

				break;
			case NO_DIR:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(RIndex)
				{
					if(LIndex)
					{
						if(LCursor != -1)
						{
							NCursor = NIndex+LCursor;
							LCursor = 0;
						}
						mem_copy(NBuf+NIndex, LBuf, LIndex);
						mem_copy(NBuf+MaxSize-NIndex-LIndex, LBuf, LIndex);
						NIndex += LIndex;
						LIndex = 0;
					}
					if(Cursor == i)
						NCursor = NIndex;
					mem_copy(NBuf+NIndex, Buf, s);
					mem_copy(NBuf+MaxSize-NIndex-s, Buf, s);
					NIndex += s;
				}
				else
				{
					if(Cursor == i)
					{
						*pCursor = pDst-pOldDst;
						Cursor = -1;
					}
					mem_copy(pDst, Buf, s);
					pDst += s;
				}

				break;
			case NONE:

				c = GetLinked(c);
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}
				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case BEFORE:

				c = GetLinked(c);
				if(Str[i].PrevType == DUAL)
					c += FINAL;
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}
				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case DUAL:

				c = GetLinked(c);
				pt = Str[i].PrevType;
				nt = Str[i].NextType;
				if(pt == DUAL)
					if(nt == BEFORE || nt == DUAL)
						c += MEDIAL;
					else
						c += FINAL;
				else
					if(nt == BEFORE || nt == DUAL)
						c += INITIAL;
				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}
				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			case NO_BREAK:

				s = str_utf8_encode(Buf, c);
				if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
					break;

				if(NIndex)
				{
					if(NCursor != -1)
					{
						RCursor = RIndex+NCursor;
						NCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-NIndex, NBuf+MaxSize-NIndex, NIndex);
					RIndex += NIndex;
					NIndex = 0;
				}
				if(LIndex)
				{
					if(LCursor != -1)
					{
						RCursor = RIndex+LIndex-LCursor;
						LCursor = 0;
					}
					mem_copy(RBuf+MaxSize-RIndex-LIndex, LBuf, LIndex);
					RIndex += LIndex;
					LIndex = 0;
				}

				if(Cursor == i)
					RCursor = RIndex;
				mem_copy(RBuf+MaxSize-RIndex-s, Buf, s);
				RIndex += s;

				break;
			}
		}
		if(s > pDstMax-pDst-RIndex-NIndex-LIndex)
			break;
	}
	if(FullRTL)
	{
		if(NIndex)
		{
			if(NCursor != -1)
			{
				*pCursor = pDst-pOldDst+NIndex-NCursor;
				Cursor = -1;
			}
			mem_copy(pDst, NBuf+MaxSize-NIndex, NIndex);
			pDst += NIndex;
		}
		if(LIndex)
		{
			if(LCursor != -1)
			{
				*pCursor = pDst-pOldDst+LCursor;
				Cursor = -1;
			}
			mem_copy(pDst, LBuf, LIndex);
			pDst += LIndex;
		}
		if(RIndex)
		{
			if(RCursor != -1)
			{
				*pCursor = pDst-pOldDst+RIndex-RCursor;
				Cursor = -1;
			}
			mem_copy(pDst, RBuf+MaxSize-RIndex, RIndex);
			pDst += RIndex;
		}
		if(Cursor != -1)
			*pCursor = 0;
	}
	else
	{
		if(RIndex)
		{
			if(RCursor != -1)
			{
				*pCursor = pDst-pOldDst+RIndex-RCursor;
				Cursor = -1;
			}
			mem_copy(pDst, RBuf+MaxSize-RIndex, RIndex);
			pDst += RIndex;
		}
		if(NIndex)
		{
			if(NCursor != -1)
			{
				*pCursor = pDst-pOldDst+NCursor;
				Cursor = -1;
			}
			mem_copy(pDst, NBuf, NIndex);
			pDst += NIndex;
		}
		if(LIndex)
		{
			if(LCursor != -1)
			{
				*pCursor = pDst-pOldDst+LCursor;
				Cursor = -1;
			}
			mem_copy(pDst, LBuf, LIndex);
			pDst += LIndex;
		}
		if(Cursor != -1)
			*pCursor = pDst-pOldDst;
	}

	*pDst = 0;	// null-terminate the string
	*pFixedLen = pDst-pOldDst+1;
}

int CRTLFix::GetLinkType(int Char)
{
	for(int i = 0; i < RTL_RANGE_N*2; i+=2)
		if(Char >= RTL_RANGE[i] && Char <= RTL_RANGE[i+1])
		{
			// Sorted by usage (better performence)
			if(Char >= ARABIC_CHARS_RANGE[0] && Char <= ARABIC_CHARS_RANGE[1])
				return ARABIC_CHARS_TYPE[Char-ARABIC_CHARS_RANGE[0]];
			for(int i = 0; i < PERSIAN_CHARS_N; i++)
				if(Char == PERSIAN_CHARS[i])
					return PERSIAN_CHARS_TYPE[i];
			for(int i = 0; i < RTL_CHARS_N; i++)
				if(Char == RTL_CHARS[i])
					return NONE;
			for(int i = 0; i < RTL_CHARS_RANGE_N*2; i+=2)
				if(Char >= RTL_CHARS_RANGE[i] && Char <= RTL_CHARS_RANGE[i+1])
					return NONE;
		}
	for(int i = 0; i < NUMBERS_RANGE_N*2; i+=2)
		if(Char >= NUMBERS_RANGE[i] && Char <= NUMBERS_RANGE[i+1])
			return NUMBER;
	for(int i = 0; i < NO_DIR_CHARS_RANGE_N*2; i+=2)
		if(Char >= NO_DIR_CHARS_RANGE[i] && Char <= NO_DIR_CHARS_RANGE[i+1])
			return NO_DIR;
	for(int i = 0; i < NO_DIR_CHARS_N; i++)
		if(Char == NO_DIR_CHARS[i])
			return NO_DIR;
	for(int i = 0; i < NO_BREAK_CHARS_RANGE_N*2; i+=2)
		if(Char >= NO_BREAK_CHARS_RANGE[i] && Char <= NO_BREAK_CHARS_RANGE[i+1])
			return NO_BREAK;
	for(int i = 0; i < NO_BREAK_CHARS_N; i++)
		if(Char == NO_BREAK_CHARS[i])
			return NO_BREAK;
	return LTR;
}

int CRTLFix::GetLinked(int Char)
{
	if(Char >= ARABIC_CHARS_RANGE[0] && Char <= ARABIC_CHARS_RANGE[1])
		return ARABIC_CHARS_LINK[Char-ARABIC_CHARS_RANGE[0]];
	for(int i = 0; i < PERSIAN_CHARS_N; i++)
		if(Char == PERSIAN_CHARS[i])
			return PERSIAN_CHARS_LINK[i];
	// Retrun others as the same (also LAMALEF)
	return Char;
}