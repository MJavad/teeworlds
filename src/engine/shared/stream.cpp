#include "stream.h"
#include <base/system.h>
#include <base/math.h>

CStream::CStream()
{
    m_pFirst = 0;
    m_Size = 0;
    m_Lock = lock_create();
}

CStream::~CStream()
{
    lock_wait(m_Lock);
    while(m_pFirst)
    {
        CStreamBlock *pLast = m_pFirst;
        m_pFirst = m_pFirst->m_pNext;

        delete []pLast->m_pData;
        delete pLast;
    }
    lock_release(m_Lock);
    lock_destroy(m_Lock);
}

void CStream::Add(const char *pData, int Size)
{
    if (Size == 0)
    {
        return;
    }
    lock_wait(m_Lock);
    CStreamBlock *pNew = new CStreamBlock();

    pNew->m_pNext = 0;
    pNew->m_Start = 0;
    pNew->m_Size = Size;
    pNew->m_pData = new char[Size];
    mem_copy(pNew->m_pData, pData, Size);

    if (m_pFirst == 0)
    {
        m_pFirst = pNew;
    }
    else
    {
        CStreamBlock *pLast = m_pFirst;
        while(pLast->m_pNext)
        {
            pLast = pLast->m_pNext;
        }
        pLast->m_pNext = pNew;
    }
    m_Size += Size;
    lock_release(m_Lock);
}

int CStream::Get(char *pData, int Size)
{
    lock_wait(m_Lock);
    int ReadSize = 0;

    if (m_pFirst == 0)
    {
        lock_release(m_Lock);
        return 0;
    }

    CStreamBlock *pItem = m_pFirst;
    while(Size > 0 && pItem)
    {
        mem_copy(pData + ReadSize, pItem->m_pData + pItem->m_Start, min(pItem->m_Size, Size));
        ReadSize += min(pItem->m_Size, Size);
        Size -= min(pItem->m_Size, Size);
        pItem = pItem->m_pNext;
    }
    lock_release(m_Lock);
    return ReadSize;
}

int CStream::Get(char *pData, int Size) const
{
    lock_wait(m_Lock);
    int ReadSize = 0;

    if (m_pFirst == 0)
    {
        lock_release(m_Lock);
        return 0;
    }

    CStreamBlock *pItem = m_pFirst;
    while(Size > 0 && pItem)
    {
        mem_copy(pData + ReadSize, pItem->m_pData + pItem->m_Start, min(pItem->m_Size, Size));
        ReadSize += min(pItem->m_Size, Size);
        Size -= min(pItem->m_Size, Size);
        pItem = pItem->m_pNext;
    }
    lock_release(m_Lock);
    return ReadSize;
}

void CStream::Remove(int Size)
{
    lock_wait(m_Lock);
    if (m_pFirst == 0 || Size == 0)
    {
        lock_release(m_Lock);
        return;
    }
    m_Size -= Size;
    if (m_Size < 0)
    {
        m_Size = 0;
    }

    CStreamBlock *pItem = m_pFirst;
    while(Size > 0 && pItem)
    {
        if (Size >= pItem->m_Size)
        {
            Size = Size - pItem->m_Size;

            m_pFirst = pItem->m_pNext;
            delete []pItem->m_pData;
            delete pItem;
            pItem = m_pFirst;
            continue;
        }
        else
        {
            pItem->m_Size = pItem->m_Size - Size;
            pItem->m_Start = pItem->m_Start + Size;

            Size = 0;
        }
        pItem = pItem->m_pNext;
    }
    lock_release(m_Lock);
}

void CStream::RemoveAll()
{
    lock_wait(m_Lock);
    if (m_pFirst == 0)
    {
        lock_release(m_Lock);
        return;
    }
    m_Size = 0;
    if (m_Size < 0)
    {
        m_Size = 0;
    }

    CStreamBlock *pItem = m_pFirst;
    while(pItem)
    {
        m_pFirst = pItem->m_pNext;
        delete []pItem->m_pData;
        delete pItem;
        pItem = m_pFirst;
    }
    lock_release(m_Lock);
}

CStream & CStream::operator = (const CStream &Other)
{
    if (Other.m_Size <= 0)
        return *this;
    int Size = Other.m_Size;
    char *pNew = new char[Size];
    Other.Get(pNew, Size);

    RemoveAll();
    Add(pNew, Size);

    delete []pNew;
    return *this;
}

CStream & CStream::operator += (const CStream &Other)
{
    if (Other.m_Size <= 0)
        return *this;
    if (m_Size == 0)
    {
        *this = Other;
        return *this;
    }
    int Size = Other.m_Size;
    char *pNew = new char[Size];
    Other.Get(pNew, Size);

    Add(pNew, Size);

    delete []pNew;
    return *this;
}
