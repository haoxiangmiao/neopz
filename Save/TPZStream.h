/**
 * @file
 * @brief Contains declaration of the abstract TPZStream class. TPZStream defines the interface for saving and reading data,\n
 * TPZFileStream implements reading from and writing to an ascii file.\n
 * TPZBFileStream implements reading from and writing to a binary file.\n
 * Finally, TPZBufferedStream implements reading from and writing to a buffer.
 */
#ifndef TPZSTREAM_H
#define TPZSTREAM_H

#include <stddef.h>          // for NULL
#include <complex>           // for complex
#include <fstream>           // for string
#include <map>               // for map
#include <set>               // for set
#include <type_traits>       // for enable_if, is_same
#include <vector>            // for vector
#include "TPZSavable.h"     // for TPZSavable
#include "pzadmchunk.h"      // for TPZAdmChunkVector
#include "pzchunk.h"         // for TPZChunkVector
#include "pzmanvector.h"     // for TPZManVector
#include "pzreal.h"          // for REAL, TPZFlopCounter, is_arithmetic_pz
#include "pzvec.h"           // for TPZVec
#include "tpzautopointer.h"  // for TPZAutoPointer
#include "TPZPersistenceManager.h"


#ifdef _AUTODIFF
template <class T> class Fad;
#endif

static unsigned long fCurrentVersion = 1; //TODO:AQUIFRANTake this away

#define TPZostream std::ostream

/**
 * @ingroup save
 * @brief Defines the interface for saving and reading data. \ref save "Persistency"
 */

/**
 In fact, this class could use the facilities of the stream class of the std library
 This class is a subset of the functionality of the stream classes
 */
class TPZStream {
public:

    TPZStream() {
    }

    virtual ~TPZStream() {
    }

    virtual void Write(const bool val);

    virtual void Write(const int *p, int howMany = 1) = 0;

    virtual void Write(const unsigned int *p, int howMany = 1) = 0;

    virtual void Write(const long unsigned int *p, int howMany = 1) = 0;

    virtual void Write(const long *p, int howMany = 1) = 0;

    virtual void Write(const float *p, int howMany = 1) = 0;

    virtual void Write(const double *p, int howMany = 1) = 0;

    virtual void Write(const long double *p, int howMany = 1) = 0;

    virtual void Write(const char *p, int howMany = 1) = 0;

    virtual void Write(const std::string *p, int howMany = 1);

    virtual void Write(const std::complex< float > *p, int howMany = 1) = 0;

    virtual void Write(const std::complex< double > *p, int howMany = 1) = 0;

    virtual void Write(const std::complex< long double > *p, int howMany = 1) = 0;

#ifdef _AUTODIFF

    virtual void Write(const Fad< float > *p, int howMany = 1) = 0;

    virtual void Write(const Fad< double > *p, int howMany = 1) = 0;

    virtual void Write(const Fad< long double > *p, int howMany = 1) = 0;

#endif

#ifndef ELLIPS
    void Write(const TPZFlopCounter *p, int howMany = 1);
#endif

    virtual void Read(bool &val);

    virtual void Read(int *p, int howMany = 1) = 0;

    virtual void Read(unsigned int *p, int howMany = 1) = 0;

    virtual void Read(long unsigned int *p, int howMany = 1) = 0;

    virtual void Read(long *p, int howMany = 1) = 0;

    virtual void Read(float *p, int howMany = 1) = 0;

    virtual void Read(double *p, int howMany = 1) = 0;

    virtual void Read(long double *p, int howMany = 1) = 0;

    virtual void Read(char *p, int howMany = 1) = 0;

    virtual void Read(std::string *p, int howMany = 1);

    virtual void Read(std::complex< float > *p, int howMany = 1) = 0;

    virtual void Read(std::complex< double > *p, int howMany = 1) = 0;

    virtual void Read(std::complex< long double > *p, int howMany = 1) = 0;

#ifdef _AUTODIFF

    virtual void Read(Fad< float > *p, int howMany = 1) = 0;

    virtual void Read(Fad< double > *p, int howMany = 1) = 0;

    virtual void Read(Fad< long double > *p, int howMany = 1) = 0;

#endif

#ifndef ELLIPS
    void Read(TPZFlopCounter *p, int howMany = 1);
#endif

    //VECTORS AND ARRAYS

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const TPZVec<T> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
    }

    template <class T,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const TPZVec<T> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++) {
            vec[c].Write(*this, 0);
        }
    }
    
    template <class T>
    void WritePointers(const TPZVec<TPZAutoPointer<T>> &vec) {
        unsigned long size = vec.NElements();
        this->Write(&size);
        for (unsigned long i = 0; i < size; ++i) {
            TPZPersistenceManager::WritePointer(vec[i].operator ->(), this);
        }
    }

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const std::vector<T> &vec) {
        int nel = vec.size();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.size());
    }

    template <class T,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const std::vector<T> &vec) {
        int c, nc = vec.size();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
    }

    void Write(const TPZVec<TPZFlopCounter> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        TPZVec<REAL> temp(nel);
        for (int iel = 0; iel < nel; iel++) {
            temp[iel] = vec[iel];
        }
        if (nel) this->Write(&temp[0], vec.NElements());
    }

    template <class T, int EXP>
    void Write(const TPZChunkVector<T, EXP> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
    }

    template <class T, int EXP,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const TPZAdmChunkVector<T, EXP> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
        this->Write(&vec.fCompactScheme);
        Write(vec.fFree);
        Write(vec.fNFree);
    }

    template <class T, int EXP,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const TPZAdmChunkVector<T, EXP> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            this->Write(&vec[c]);
        this->Write(&vec.fCompactScheme);
        Write(vec.fFree, true);
        Write(vec.fNFree, true);
    }

    template <class T, class U> void Write(const std::map<T, U> &vec) {
        long sz = vec.size();
        TPZManVector<T> keyVec(sz);
        TPZManVector<U> valVec(sz);
        long count = 0;
        typename std::map<T, U>::const_iterator it;
        for (it = vec.begin(); it != vec.end(); it++) {
            keyVec[count] = it->first;
            valVec[count++] = it->second;
        }
        Write(keyVec);
        Write(valVec);
    }

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Write(const std::set<T> &vec) {
        int nel = vec.size();
        this->Write(&nel);
        typename std::set<T>::iterator it = vec.begin();
        while (it != vec.end()) {
            int val = *it;
            this->Write(&val);
            it++;
        }
    }

    template <class T>
    void Read(TPZVec<T> &vec) {
        Read(vec, NULL);
    }

    template <class T,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(TPZVec<T> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c].Read(*this, context);
        }
    }

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(TPZVec<T> &vec, void *context) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
    }
    
    
    template <class T>
    void ReadPointers(TPZVec<TPZAutoPointer<T>> &vec) {
        unsigned long size;
        this->Read(&size,1);
        vec.resize(size);
        for (unsigned long i = 0; i < size; ++i) {
            vec[i] = TPZAutoPointerDynamicCast<T>(TPZPersistenceManager::GetAutoPointer(this));
        }
    }

    void Read(TPZVec<TPZFlopCounter> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        TPZVec<REAL> temp(nc);
        if (nc) this->Read(&temp[0], nc);
        for (long ic = 0; ic < nc; ic++) {
            vec[ic] = temp[ic];
        }
    }

    template <class T,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(std::vector<T> &vec, void *context) {
        int c, nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c].Read(*this, context);
        }
    }

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(std::vector<T> &vec, void *context) {
        int nel;
        this->Read(&nel, 1);
        for (int i = 0; i < nel; i++) {
            int val;
            this->Read(&val);
            vec.insert(val);
        }
    }

    template <int N>
    void Read(TPZManVector<REAL, N> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
    }

    template <class T, int EXP>
    void Read(TPZChunkVector<T, EXP> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c].Read(*this, context);
        }
    }

    template <class T, int EXP,
    typename std::enable_if<!(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(TPZAdmChunkVector<T, EXP> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++)
            vec[c].Read(*this, context);
        this->Read(&vec.fCompactScheme, 1);
        Read(vec.fFree);
        Read(vec.fNFree);
    }

    template <class T, class U> void Read(std::map<T, U> &vec) {
        TPZManVector<T> keyVec;
        this->Read(keyVec);
        TPZManVector<U> valVec;
        this->Read(valVec);
        int sz = keyVec.NElements();
#ifdef PZDEBUG
        if (sz != valVec.NElements()) {
            DebugStop();
        }
#endif
        int i;
        for (i = 0; i < sz; ++i) {
            vec[keyVec[i]] = valVec[i];
        }
    }

    template <class T,
    typename std::enable_if<(std::is_same<std::string, T>::value || is_arithmetic_pz<T>::value), int>::type* = nullptr>
    void Read(std::set<T> &vec) {
        int nel;
        this->Read(&nel, 1);
        for (int i = 0; i < nel; i++) {
            T val;
            this->Read(&val);
            vec.insert(val);
        }
    }
    ////////////////

    template <class T>
    void WritePointers(const TPZVec<T *> &vec);
    
    template <class T>
    void ReadPointers(TPZVec<T *> &vec);

    template <class T>
    void WritePointers(const std::map<int, TPZAutoPointer<T>> &vec);
    
    template <class T>
    void ReadPointers(std::map<int, TPZAutoPointer<T> > &vec);
    
    template <class T>
    void WritePointers(const std::map<int, T *> &vec);
    
    template <class T>
    void ReadPointers(std::map<int, T *> &vec);
    
    template <class T>
    void WritePointers(const std::set<T *> &vec);
    
    template <class T>
    void ReadPointers(std::set<T *> &vec);

    template <class T, int EXP>
    void WritePointers(const TPZChunkVector<T *, EXP> &vec);
    
    template <class T, int EXP>
    void ReadPointers(TPZChunkVector<T *, EXP> &vec);

    template <class T, int EXP>
    void WritePointers(const TPZAdmChunkVector<T *, EXP> &vec);

    template <class T, int EXP>
    void ReadPointers(TPZAdmChunkVector<T *, EXP> &vec);

protected:

};

#include "TPZPersistenceManager.h"
template <class T> void TPZStream::WritePointers(const TPZVec<T *> &vec) {
    long unsigned int nObjects = vec.NElements();
    this->Write(&nObjects);
    for (long i = 0; i < nObjects; i++) {
        TPZPersistenceManager::WritePointer(vec[i], this);
    }
}

template <class T> void TPZStream::ReadPointers(TPZVec<T *> &vec) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    vec.Resize(nObjects);
    for (long unsigned int i = 0; i < nObjects; ++i) {
        vec[i] = dynamic_cast<T *>(TPZPersistenceManager::GetInstance(this));
    }
}

template <class T>
void TPZStream::WritePointers(const std::map<int, TPZAutoPointer<T>> &map) {
    long unsigned int nObjects = map.size();
    this->Write(&nObjects);
    typedef typename std::map<int, TPZAutoPointer<T>>::const_iterator map_it;
    map_it it;
    for (it = map.begin(); it != map.end(); it++) {
        this->Write(&(it->first));
        TPZPersistenceManager::WritePointer(it->second, this);
    }
}

template <class T>
void TPZStream::ReadPointers(std::map<int, TPZAutoPointer<T>> &map) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    int key;
    for (long unsigned int i = 0; i < nObjects; ++i) {
        Read(&key);
        map[key] = dynamic_cast<TPZAutoPointer<T>>(
                                                   TPZPersistenceManager::GetAutoPointer(this));
    }
}

template <class T>
void TPZStream::WritePointers(const std::map<int, T *> &vec) {
    long unsigned int nObjects = vec.size();
    this->Write(&nObjects);
    typedef typename std::map<int, T *>::const_iterator vec_it;
    vec_it it;
    for (it = vec.begin(); it != vec.end(); it++) {
        this->Write(&it->first);
        TPZPersistenceManager::WritePointer(it->second, this);
    }
}

template <class T> void TPZStream::ReadPointers(std::map<int, T *> &map) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    int key;
    for (long unsigned int i = 0; i < nObjects; ++i) {
        Read(&key);
        map[key] = dynamic_cast<T *>(TPZPersistenceManager::GetInstance(this));
    }
}

template <class T> void TPZStream::WritePointers(const std::set<T *> &_set) {
    long unsigned int nObjects = _set.size();
    this->Write(&nObjects);
    typedef typename std::set<T *>::const_iterator _set_it;
    _set_it it;
    while (it != _set.end()) {
        TPZPersistenceManager::WritePointer(it, this);
        it++;
    }
}

template <class T> void TPZStream::ReadPointers(std::set<T *> &_set) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    
    for(long unsigned int i = 0; i < nObjects; ++i) {
        _set.insert(dynamic_cast<T *>(TPZPersistenceManager::GetInstance(this)));
    }
}

template <class T, int EXP>
void TPZStream::WritePointers(const TPZChunkVector<T *, EXP> &vec) {
    long unsigned int nObjects = vec.NElements();
    this->Write(&nObjects);
    for (long unsigned int i = 0; i < nObjects; i++) {
        TPZPersistenceManager::WritePointer(vec[i], this);
    }
}

template <class T, int EXP>
void TPZStream::ReadPointers(TPZChunkVector<T *, EXP> &vec) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    vec.Resize(nObjects);
    for (long unsigned int i = 0; i < nObjects; ++i) {
        vec[i] = dynamic_cast<T *>(TPZPersistenceManager::GetInstance(this));
    }
}

template <class T, int EXP>
void TPZStream::WritePointers(const TPZAdmChunkVector<T *, EXP> &vec) {
    long unsigned int nObjects = vec.NElements();
    this->Write(&nObjects);
    for (long unsigned int i = 0; i < nObjects; i++) {
        TPZPersistenceManager::WritePointer(vec[i], this);
    }
    this->Write(&vec.fCompactScheme);
    Write(vec.fFree);
    Write(vec.fNFree);
}

template <class T, int EXP>
void TPZStream::ReadPointers(TPZAdmChunkVector<T *, EXP> &vec) {
    long unsigned int nObjects;
    this->Read(&nObjects);
    vec.Resize(nObjects);
    for (long unsigned int i = 0; i < nObjects; ++i) {
        vec[i] = dynamic_cast<T *>(TPZPersistenceManager::GetInstance(this));
    }
    Read(&vec.fCompactScheme);
    Read(vec.fFree);
    Read(vec.fNFree);
}

#endif//TPZSTREAM_H