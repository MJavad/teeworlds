#ifndef RTL_SUPPORT_H
#define RTL_SUPPORT_H

enum
{
	// Letter contexts
	ISOLATED = 0,
	FINAL,
	INITIAL,
	MEDIAL,

	// Link types
	LTR = 0,
	NUMBER,
	NO_DIR,
	NONE,
	BEFORE,
	DUAL,
	NO_BREAK
};

const int RTL_RANGE[] = { 0x060C, 0x06F9, 0xFB52, 0xFEFC };
const int RTL_RANGE_N = 2;

const int ARABIC_CHARS_RANGE[] = { 0x0621, 0x064A };
const int ARABIC_CHARS_TYPE[] = {
    NONE, 		BEFORE, 	BEFORE,		BEFORE,		BEFORE,		DUAL,		BEFORE,		DUAL,
    BEFORE,		DUAL,		DUAL,		DUAL,		DUAL,		DUAL,		BEFORE,		BEFORE,
    BEFORE,		BEFORE,		DUAL,		DUAL,		DUAL,		DUAL,		DUAL,		DUAL,
    DUAL,		DUAL,		NONE,		NONE,		NONE,		NONE,		NONE,		NO_BREAK,
    DUAL,		DUAL,		DUAL,		DUAL,		DUAL,		DUAL,		DUAL,		BEFORE,
    DUAL,		DUAL		};

const int ARABIC_CHARS_LINK[] = {
	0xFE80,		0xFE81,		0xFE83,		0xFE85,		0xFE87,		0xFE89,		0xFE8D,		0xFE8F,
	0xFE93,		0xFE95,		0xFE99,		0xFE9D,		0xFEA1,		0xFEA5,		0xFEA9,		0xFEAB,
	0xFEAD,		0xFEAF,		0xFEB1,		0xFEB5,		0xFEB9,		0xFEBD,		0xFEC1,		0xFEC5,
	0xFEC9,		0xFECD,		0x063B,		0x063C,		0x063D,		0x063E,		0x063F,		0x0640,
	0xFED1,		0xFED5,		0xFED9,		0xFEDD,		0xFEE1,		0xFEE5,		0xFEE9,		0xFEED,
	0xFEEF,		0xFEF1		};

const int ARABIC_LAM = 0x0644;
const int ARABIC_ALEF_CHARS[] = {
	0x0622,		0x0623,		0x0625,		0x0627		};
const int ARABIC_ALEF_CHARS_N = 4;
const int ARABIC_LAMALEF_CHARS[] = {	// Link is the same
	0xFEF5,		0xFEF7,		0xFEF9,		0xFEFB		};
const int ARABIC_LAMALEF_CHARS_N = 4;


const int PERSIAN_CHARS[] = {
	0x067E,		0x0686,		0x0698,		0x06A9,		0x06AF,		0x06CC		};

const int PERSIAN_CHARS_TYPE[] = {
	DUAL,		DUAL,		BEFORE,		DUAL,		DUAL,		DUAL		};

const int PERSIAN_CHARS_LINK[] = {
	0xFB56,		0xFB7A,		0xFB8A,		0xFB8E,		0xFB92,		0xFBFC		};

const int PERSIAN_CHARS_N = 6;

const int RTL_CHARS[] = { 0x060C, 0x061B, 0x061F };
const int RTL_CHARS_N = 3;
const int RTL_CHARS_RANGE[] = { 0 };
const int RTL_CHARS_RANGE_N = 0;

const int NUMBERS_RANGE[] = { 0x0030, 0x0039, 0x0660, 0x066D, 0x06F0, 0x006F9 };
const int NUMBERS_RANGE_N = 3;

const int NO_BREAK_CHARS[] = { 0x00A0, 0x0615, 0x0670, 0x0674, 0x202F, 0xFEFF };
const int NO_BREAK_CHARS_N = 5;
const int NO_BREAK_CHARS_RANGE[] = { 0x064B, 0x065A };
const int NO_BREAK_CHARS_RANGE_N = 1;

const int NO_DIR_CHARS[] = { 0x0020, 0x005C, 0x005F };
const int NO_DIR_CHARS_N = 2;
const int NO_DIR_CHARS_RANGE[] = { 0x0021, 0x002F, 0x003A, 0x003F, 0x005B, 0x0060, 0x007B, 0x007E, 0x00A1, 0x00B1 };
const int NO_DIR_CHARS_RANGE_N = 5;

const int MAX_RTL_FIX_SRT_LEN = 1024; // it means max of 1024 utf8 chars not 1024 bytes

class CRTLFix
{
	struct CChar
	{
		int PrevType;
		int Type;
		int NextType;
		int Char;
	};
	int GetLinkType(int Char);
	int GetLinked(int Char);
public:
	void FixString(char *pDst, const char *pSrc, int DstSize, bool FullRTL, int *pCursor = 0, int *pFixedLen = 0);
};

#endif
