/*---------------------------------------------------------------------------*\

    Copyright (C) 2000 AVM GmbH. All rights reserved.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
    ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
    with the use or performance of this software. See the
    GNU General Public License for further details.

\*---------------------------------------------------------------------------*/

#ifndef _PROTCLAS_H_
#define _PROTCLAS_H_

#define USE_TEMPLATE_FUNCTIONALITY

#include "Protect.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define UNIQUE_INIT_VALUE           (tUInt)0

typedef tBool (*tProtectUniqueFunc) (void *);

class CProtectUniqueClass {
public:
    CProtectUniqueClass (void) : m_Counter (UNIQUE_INIT_VALUE) {}
    virtual ~CProtectUniqueClass (void) {}

    virtual tBool SynchronFunc (void) = 0;

private:
    tUInt m_Counter;

    friend class CProtectClass;
    friend void basicUniqueCall (CProtectUniqueClass *pPUC);
};


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#ifdef USE_TEMPLATE_FUNCTIONALITY

template <class T> inline tBool basicTestAndSet (T *pVar, T testVal, T setVal) {
    if (*pVar != testVal) {
        return vFalse;
    }
    *pVar = setVal;
    return vTrue;
}

template <class T> inline T basicInc (T *pVar) {
    tUInt fret = *pVar;
    (*pVar)++;
    return fret;
}

template <class T> inline T basicDec (T *pVar) {
    tUInt fret = *pVar;
    (*pVar)--;
    return fret;
}

#else

inline tBool basicTestAndSet (tUInt *pVar, tUInt testVal, tUInt setVal) {
    if (*pVar != testVal) {
        return vFalse;
    }
    *pVar = setVal;
    return vTrue;
}

inline tUInt basicInc (tUInt *pVar) {
    tUInt fret = *pVar;
    (*pVar)++;
    return fret;
}

inline tUInt basicDec (tUInt *pVar) {
    tUInt fret = *pVar;
    (*pVar)--;
    return fret;
}

#endif
/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool basicUniqueStart (tUInt *pCounter) {
    (*pCounter)++;
    return (*pCounter) == UNIQUE_INIT_VALUE + 1;
}

inline tBool basicUniqueStop (tUInt *pCounter) {
    (*pCounter)--;
    return (*pCounter) == UNIQUE_INIT_VALUE;
}

inline void basicUniqueClear (tUInt *pCounter) {
    *pCounter = UNIQUE_INIT_VALUE;
}

inline void basicUniqueCall (tUInt *pCounter, tProtectUniqueFunc Func, void *Handle) {
    if (basicUniqueStart (pCounter)) {
        do {
            if (Func (Handle) == vFalse) {
                basicUniqueClear (pCounter);
                return;
            }
        } while (basicUniqueStop (pCounter) == vFalse);
    }
}

inline void basicUniqueCall (CProtectUniqueClass *pPUC) {
    if (basicUniqueStart (&pPUC->m_Counter)) {
        do {
            if (pPUC->SynchronFunc() == vFalse) {
                basicUniqueClear (&pPUC->m_Counter);
                return;
            }
        } while (basicUniqueStop (&pPUC->m_Counter) == vFalse);
    }
}


/*===========================================================================*\
\*===========================================================================*/

class CProtectClass {
public:
    CProtectClass (void);
    virtual ~CProtectClass (void);
    tBool Init (void);          // Init can be called multiple times!!
    tBool IsInitialized (void); // Mainly for debuging purpose

    void BeginRead (void);
    void EndRead (void);
    void BeginWrite (void);
    void EndWrite (void);

    // Set, Add, Sub, Inc & Dec return always the old/previous value of pVar!!
#ifdef USE_TEMPLATE_FUNCTIONALITY
    template <class T> T Get (T *pVar) {
        BeginRead();
        T fret = *pVar;
        EndRead();
        return fret;
    }

    template <class T> T Set (T *pVar, T value) {
        BeginWrite();
        T fret = *pVar;
        *pVar  = value;
        EndWrite();
        return fret;
    }

    template <class T> tBool TestAndSet (T *pVar, T testVal, T setVal) {
        tBool fret = vFalse;
        BeginWrite();
        if (*pVar == testVal) {
            *pVar = setVal;
            fret  = vTrue;
        }
        EndWrite();
        return fret;
    }

    template <class T> T Add (T *pVar, T value) {
        BeginWrite();
        T fret = *pVar;
        *pVar += value;
        EndWrite();
        return fret;
    }

    template <class T> T Sub (T *pVar, T value) {
        BeginWrite();
        T fret = *pVar;
        *pVar -= value;
        EndWrite();
        return fret;
    }

    template <class T> T Inc (T *pVar) {
        BeginWrite();
        T fret = *pVar;
        (*pVar)++;
        EndWrite();
        return fret;
    }

    template <class T> T Dec (T *pVar) {
        BeginWrite();
        T fret = *pVar;
        (*pVar)--;
        EndWrite();
        return fret;
    }

#else
    tUInt Get (tUInt *pVar);
    tBool Get (tBool *pVar);
    tUInt Set (tUInt *pVar, tUInt value);
    tBool Set (tBool *pVar, tBool value);
    tBool TestAndSet (tUInt *pVar, tUInt testVal, tUInt setVal);

    tUInt Add (tUInt *pVar, tUInt value);
    tUInt Sub (tUInt *pVar, tUInt value);

    tUInt Inc (tUInt *pVar);
    tUInt Dec (tUInt *pVar);
#endif

    tBool UniqueStart (tUInt *pCounter);
    tBool UniqueStop (tUInt *pCounter);
    void  UniqueClear (tUInt *pCounter);
    void  UniqueCall (tUInt *pCounter, tProtectUniqueFunc Func, void *Handle);
    void  UniqueCall (CProtectUniqueClass *pPUC);

private:
    Protect_Define (m_hProtect);
};


/*===========================================================================*\
    CProtectClass - inline functions
\*===========================================================================*/

inline CProtectClass::CProtectClass (void) {
    Protect_InitHandle (&m_hProtect);
    (void)Protect_Create (&m_hProtect);
}

inline CProtectClass::~CProtectClass (void) {
    Protect_Destroy (&m_hProtect);
}

inline tBool CProtectClass::Init (void) {
    return Protect_IsCreated (m_hProtect) ? vTrue : Protect_Create (&m_hProtect);
}

inline tBool CProtectClass::IsInitialized (void) {
    return Protect_IsCreated (m_hProtect) ? vTrue : vFalse;
}

inline void CProtectClass::BeginRead (void) {
    Protect_BeginRead (m_hProtect);
}

inline void CProtectClass::EndRead (void) {
    Protect_EndRead (m_hProtect);
}

inline void CProtectClass::BeginWrite (void) {
    Protect_BeginWrite (m_hProtect);
}

inline void CProtectClass::EndWrite (void) {
    Protect_EndWrite (m_hProtect);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#ifndef USE_TEMPLATE_FUNCTIONALITY

inline tUInt CProtectClass::Get (tUInt *pVar) {
    BeginRead();
    tUInt fret = *pVar;
    EndRead();
    return fret;
}

inline tBool CProtectClass::Get (tBool *pVar) {
    BeginRead();
    tBool fret = *pVar;
    EndRead();
    return fret;
}

inline tUInt CProtectClass::Set (tUInt *pVar, tUInt value) {
    BeginWrite();
    tUInt fret = *pVar;
    *pVar  = value;
    EndWrite();
    return fret;
}

inline tBool CProtectClass::Set (tBool *pVar, tBool value) {
    BeginWrite();
    tBool fret = *pVar;
    *pVar  = value;
    EndWrite();
    return fret;
}

inline tBool CProtectClass::TestAndSet (tUInt *pVar, tUInt testVal, tUInt setVal) {
    tBool fret = vFalse;
    BeginWrite();
    if (*pVar == testVal) {
        *pVar = setVal;
        fret  = vTrue;
    }
    EndWrite();
    return fret;
}

inline tUInt CProtectClass::Add (tUInt *pVar, tUInt value) {
    BeginWrite();
    tUInt fret = *pVar;
    *pVar += value;
    EndWrite();
    return fret;
}

inline tUInt CProtectClass::Sub (tUInt *pVar, tUInt value) {
    BeginWrite();
    tUInt fret = *pVar;
    *pVar -= value;
    EndWrite();
    return fret;
}

inline tUInt CProtectClass::Inc (tUInt *pVar) {
    BeginWrite();
    tUInt fret = *pVar;
    (*pVar)++;
    EndWrite();
    return fret;
}

inline tUInt CProtectClass::Dec (tUInt *pVar) {
    BeginWrite();
    tUInt fret = *pVar;
    (*pVar)--;
    EndWrite();
    return fret;
}

#endif // !USE_TEMPLATE_FUNCTIONALITY
/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CProtectClass::UniqueStart (tUInt *pCounter) {
    return Inc (pCounter) == UNIQUE_INIT_VALUE;
}

inline tBool CProtectClass::UniqueStop (tUInt *pCounter) {
    return Dec (pCounter) == UNIQUE_INIT_VALUE + 1;
}

inline void CProtectClass::UniqueClear (tUInt *pCounter) {
    Set (pCounter, UNIQUE_INIT_VALUE);
}

inline void CProtectClass::UniqueCall (tUInt *pCounter, tProtectUniqueFunc Func, void *Handle) {
    if (UniqueStart (pCounter)) {
        do {
            if (Func (Handle) == vFalse) {
                UniqueClear (pCounter);
                return;
            }
        } while (UniqueStop (pCounter) == vFalse);
    }
}

inline void CProtectClass::UniqueCall (CProtectUniqueClass *pPUC) {
    if (UniqueStart (&pPUC->m_Counter)) {
        do {
            if (pPUC->SynchronFunc() == vFalse) {
                UniqueClear (&pPUC->m_Counter);
                return;
            }
        } while (UniqueStop (&pPUC->m_Counter) == vFalse);
    }
}


/*===========================================================================*\
\*===========================================================================*/

#endif



