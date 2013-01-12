//Copyright (c) by MAP94 2012

#ifndef ENGINE_SHARED_STREAM_H
#define ENGINE_SHARED_STREAM_H
#include <base/system.h>

struct CStreamBlock
{
    char *m_pData;
    int m_Start;
    int m_Size;
    CStreamBlock *m_pNext;
};

class CStream
{
private:
    CStreamBlock *m_pFirst;
    LOCK m_Lock;

    int m_Size;
public:
    CStream();
    ~CStream();

    void Add(const char *pData, int Size);
    int Get(char *pData, int Size) const;
    int Get(char *pData, int Size);
    void Remove(int Size);
    void RemoveAll();

    inline int GetSize() { return m_Size; }

    CStream & operator = (const CStream &Other);
    CStream & operator += (const CStream &Other);

};

#endif
