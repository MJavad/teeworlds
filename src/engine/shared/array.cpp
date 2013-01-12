#ifndef SHARED_ARRAY_CPP
#define SHARED_ARRAY_CPP
#include "array.h"

template <class T>
CArray<T>::CArray()
{
    m_Lock = lock_create();
    m_pFirst = 0;
    m_pLast = 0;
    m_Size = 0;
    m_pSelectedItem = 0;
    m_SelectedIndex = 0;
}

template <class T>
CArray<T>::~CArray()
{
    lock_wait(m_Lock);
    CArrayItem *pItem = m_pFirst;
    for (long long i = 0; i < m_Size && pItem; i++)
    {
        CArrayItem *pDel = pItem;
        pItem = pItem->m_pNext;
        delete pDel;
    }
    lock_release(m_Lock);
    lock_destroy(m_Lock);
}

template <class T>
void CArray<T>::Insert(const T &Value, long long BeforeIndex)
{
    lock_wait(m_Lock);
    if (BeforeIndex > -1)
    {
        CArrayItem *pItem = m_pFirst;
        long long i = 0;
        for (; i < BeforeIndex && pItem; i++)
        {
            pItem = pItem->m_pNext;
        }
        if (i == BeforeIndex && pItem)
        {
            InsertInternal(Value, pItem);
            lock_release(m_Lock);
            return;
        }
    }
    InsertInternal(Value, m_pLast, true);
    lock_release(m_Lock);
}

template <class T>
void CArray<T>::DeleteInternal(CArrayItem *pItem)
{
    if (!pItem)
        return;
    CArrayItem *pPrev = 0;
    CArrayItem *pNext = 0;
    pPrev = pItem->m_pPrev;
    pNext = pItem->m_pNext;
    if (pPrev)
    {
        pPrev->m_pNext = pNext;
    }
    if (pNext)
    {
        pNext->m_pPrev = pPrev;
    }
    if (pItem == m_pFirst)
        m_pFirst = pNext;
    if (pItem == m_pLast)
        m_pLast = pPrev;
    delete pItem;

    m_Size--;
    m_pSelectedItem = m_pFirst;
    m_SelectedIndex = 0;
}

template <class T>
void CArray<T>::DeleteByValues(const T &Value)
{
    lock_wait(m_Lock);
    CArrayItem *pItem = m_pFirst;
    for (long long i = 0; i < m_Size && pItem; i++)
    {
        if (pItem->m_Value == Value)
        {
            DeleteInternal(pItem);
        }
        pItem = pItem->m_pNext;
    }
    lock_release(m_Lock);
}

template <class T>
void CArray<T>::DeleteByIndex(long long Index)
{
    lock_wait(m_Lock);
    CArrayItem *pItem = m_pFirst;
    for (long long i = 0; i < m_Size && pItem; i++)
    {
        if (i == Index)
        {
            DeleteInternal(pItem);
            break;
        }
        pItem = pItem->m_pNext;
    }
    lock_release(m_Lock);
}

template <class T>
void CArray<T>::DeleteBySelection()
{
    lock_wait(m_Lock);
    if (m_pSelectedItem)
    {
        DeleteInternal(m_pSelectedItem);
    }
    lock_release(m_Lock);
}

template <class T>
void CArray<T>::InsertInternal(const T &Value, CArrayItem *pItem, bool After)
{
    CArrayItem *pPrev = 0;
    CArrayItem *pNext = 0;
    CArrayItem *pNew = new CArrayItem();

    if (pItem)
    {
        if (After)
        {
            pPrev = pItem;
            pNext = pItem->m_pNext;
        }
        else
        {
            pPrev = pItem->m_pPrev;
            pNext = pItem;
        }
    }

    pNew->m_pNext = pNext;
    pNew->m_pPrev = pPrev;

    if (pPrev)
        pPrev->m_pNext = pNew;
    else
        m_pFirst = pNew;

    if (pNext)
        pNext->m_pPrev = pNew;
    else
        m_pLast = pNew;

    pNew->m_Value = Value;

    m_Size++;

    m_pSelectedItem = m_pFirst;
    m_SelectedIndex = 0;
}

template <class T>
T CArray<T>::operator[] (long long i)
{
    lock_wait(m_Lock);
    if (m_pSelectedItem == 0)
        m_pSelectedItem = m_pFirst;
    if (i == 0)
    {
        m_SelectedIndex = i;
        m_pSelectedItem = m_pFirst;
        lock_release(m_Lock);
        return m_pSelectedItem->m_Value;
    }
    else if (i == m_Size - 1)
    {
        m_SelectedIndex = i;
        m_pSelectedItem = m_pLast;
        lock_release(m_Lock);
        return m_pSelectedItem->m_Value;
    }
    while(m_Size > i && i >= 0 && m_pSelectedItem)
    {
        if (m_SelectedIndex > i)
        {
            m_pSelectedItem = m_pSelectedItem->m_pPrev;
            m_SelectedIndex--;
        }
        else if (m_SelectedIndex < i)
        {
            m_pSelectedItem = m_pSelectedItem->m_pNext;
            m_SelectedIndex++;
        }
        if (m_SelectedIndex == i)
        {
            lock_release(m_Lock);
            return m_pSelectedItem->m_Value;
        }
    }
    lock_release(m_Lock);
    return T();
}

template <class T>
void CArray<T>::ToStream(char *pOut, long long OutSize)
{
    long long Size = 0;
    if (pOut)
    {
        CArrayItem *pItem = m_pFirst;
        for (long long i = 0; i < m_Size && pItem; i++)
        {
            dbg_msg("", "%i - %i", i, pItem->m_Value);
            mem_copy(pOut, &pItem->m_Value, sizeof(T));
            pOut = pOut + sizeof(T);
            pItem = pItem->m_pNext;
            if (Size + sizeof(T) > OutSize)
                return;
        }

    }
}
#endif
