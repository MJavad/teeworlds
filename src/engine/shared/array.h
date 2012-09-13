//Copyright (c) by MAP94 2012

#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H
#include <base/system.h>

template <class T>
class CArray
{
    LOCK m_Lock;
    struct CArrayItem
    {
        T m_Value;
        CArrayItem *m_pNext;
        CArrayItem *m_pPrev;
    };



    CArrayItem *m_pFirst;
    CArrayItem *m_pLast;

    CArrayItem *m_pSelectedItem; //for fast inorder iteration
    long long m_SelectedIndex;

    long long m_Size;

    void InsertInternal(const T &Value, CArrayItem *pItem, bool After = false);
    void DeleteInternal(CArrayItem *pItem);

public:
    CArray();
    ~CArray();

    void Insert(const T &Value, long long BeforeIndex = -1);
    void DeleteByValues(const T &Value);
    void DeleteByIndex(long long Index);
    void DeleteBySelection();

    T operator[] (long long i);

    void ToStream(char *pOut, long long OutSize);
    inline long long GetSize() { return m_Size; }
};

#include "array.cpp" //include the code
#endif

