/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_LINEINPUT_H
#define GAME_CLIENT_LINEINPUT_H

#include <engine/input.h>
#include "rtl_support.h"

// line input helter
class CLineInput
{
	class CRTLFix *m_pRTLFix;
	char m_Str[256];
	int m_Len;
	int m_CursorPos;

	char m_FixedStr[512];
	int m_FixedLen;
	int m_FixedCursorPos;


public:
	bool m_DisableFullRTL;

	static bool Manipulate(IInput::CEvent e, char *pStr, int StrMaxSize, int *pStrLenPtr, int *pCursorPosPtr, bool MultiLine = false);

	class CCallback
	{
	public:
		virtual ~CCallback() {}
		virtual bool Event(IInput::CEvent e) = 0;
	};

	CLineInput();
	void Clear();
	void ProcessInput(IInput::CEvent e);
	void ProcessCharInput(char Code);
	void Set(const char *pString);
	const char *GetOrgString() const { return m_Str; }
	const char *GetString() const { return m_FixedStr; }
	int GetOrgLength() const { return m_FixedLen; }
	int GetLength() const { return m_FixedLen; }
	int GetOrgCursorOffset() const { return m_CursorPos; }
	int GetCursorOffset() const { return m_FixedCursorPos; }
	void SetCursorOffset(int Offset) { m_CursorPos = Offset > m_Len ? m_Len : Offset < 0 ? 0 : Offset; }
};

#endif
