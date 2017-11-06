/**
 * @file
 * @brief Contains declaration of the TPZStream and TPZFileStream classes. TPZStream defines the interface for saving and reading data,\n
 * TPZFileStream implements reading from and writing to an ascii file.
 */
#ifndef PZFILEBUFFERH
#define PZFILEBUFFERH
#include <iostream>
#include <fstream>
#include <string>
#include <complex>
#include <map>
#include <vector>
#include <set>

#include "pzreal.h"
#include "tpzautopointer.h"
#include "pzmanvector.h"
#include "pzadmchunk.h"

#ifdef _AUTODIFF
#include "fad.h"
#endif

#define TPZostream std::ostream

#include "pzsave.h"
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
	
	virtual ~TPZStream() {}
    
    virtual void Write(bool val)
    {
        int ival = (val == true) ? 1 : 0;
        Write(&ival);
    }
    	
	virtual void Write(const int *p, int size=1)=0;

	virtual void Write(const unsigned int *p, int size=1)=0;
    
	virtual void Write(const long *p, int size=1)=0;

	virtual void Write(const float  *p, int size=1)=0;
	
	virtual void Write(const double  *p, int size=1)=0;
	
	virtual void Write(const long double  *p, int size=1)=0;
	
	virtual void Write(const char *p, int size=1)=0;
	
	virtual void Write(const std::string *p, int size=1) = 0;
	
	virtual void Write(const std::complex< float > *p, int size=1)=0;
	
	virtual void Write(const std::complex< double > *p, int size=1)=0;
	
	virtual void Write(const std::complex< long double > *p, int size=1)=0;
    
#ifdef _AUTODIFF
    
    virtual void Write(const Fad< float > *p, int size=1)=0;
    
    virtual void Write(const Fad< double > *p, int size=1)=0;
    
    virtual void Write(const Fad< long double > *p, int size=1)=0;
    
#endif
	
#ifndef ELLIPS
	void Write(const TPZFlopCounter *p, int size=1)
	{
		int i;
		for(i=0; i<size; i++) Write(&(p[i].fVal),1);
	}
#endif
	
    virtual void Read(bool &val)
    {
        int ival;
        Read(&ival);
        val = (ival == 0) ? false : true;
    }
    
	virtual void Read(int *p, int size=1)=0;

	virtual void Read(unsigned int *p, int size=1)=0;
    
	virtual void Read(long *p, int size=1)=0;
    
	virtual void Read(float *p, int size=1)=0;
	
	virtual void Read(double *p, int size=1)=0;
	
	virtual void Read(long double *p, int size=1)=0;

	virtual void Read(std::complex< float > *p, int size=1)=0;
	
	virtual void Read(std::complex< double > *p, int size=1)=0;
	
	virtual void Read(std::complex< long double > *p, int size=1)=0;
    
#ifdef _AUTODIFF
    
    virtual void Read(Fad< float > *p, int size=1)=0;
    
    virtual void Read(Fad< double > *p, int size=1)=0;
    
    virtual void Read(Fad< long double > *p, int size=1)=0;
    
#endif
    
#ifndef ELLIPS
	void Read(TPZFlopCounter *p, int size=1)
	{
		int i;
		for(i=0; i<size; i++)
		{
			Read(&(p[i].fVal),1);
		}
	}
#endif

    virtual void Read(char *p, int size = 1) = 0;

    virtual void Read(std::string *p, int size = 1) = 0;

    template <class T> void Write(const TPZVec<T> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
    }

    template <class T> void Write(const std::vector<T> &vec) {
        int c, nc = vec.size();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
    }

    template <class T, int EXP>
    void Write(const TPZChunkVector<T, EXP> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
    }

    template <class T, int EXP> void Write(TPZAdmChunkVector<T, EXP> &vec) {
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            vec[c].Write(*this, 0);
        this->Write(&vec.fCompactScheme);
        Write(vec.fFree);
        Write(vec.fNFree);
    }

    template <class T> void Write(const std::map<T, T> &vec) {
        long sz = vec.size();
        TPZManVector<T> cp(sz * 2);
        long count = 0;
		typename std::map<T, T>::const_iterator it;
        for (it = vec.begin(); it != vec.end(); it++) {
            cp[count++] = it->first;
            cp[count++] = it->second;
        }
        Write(cp);
    }

    /**
     * Write for chunk vectors with basic elements as float, double, long
     * double, std::complex<...> .
     */
    template <class T, int EXP>
    void Write(const TPZAdmChunkVector<T, EXP> &vec, bool basic) {
        if (!basic) {
            DebugStop();
            return;
        }
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            this->Write(&vec[c]);
        this->Write(&vec.fCompactScheme);
        Write(vec.fFree, true);
        Write(vec.fNFree, true);
    }

    template <class T> void Write(const TPZVec<T> &vec, bool basic) {
        if (!basic) {
            DebugStop();
            return;
        }
        long c, nc = vec.NElements();
        this->Write(&nc);
        for (c = 0; c < nc; c++)
            this->Write(&vec[c]);
    }

    template <class T>  void Write(const std::set<T> &vec) {
        int nel = vec.size();
        this->Write(&nel);
        typename std::set<T>::iterator it = vec.begin();
        while (it != vec.end()) {
            int val = *it;
            this->Write(&val);
            it++;
        }
    }

    template <class T> void WritePointers(TPZVec<T *> &vec) {
        long c, nc = vec.NElements();
        int emptyPos = -1;
        this->Write(&nc);
        for (c = 0; c < nc; c++) {
            if (vec[c]) {
                vec[c]->Write(*this);
            } else {
                this->Write(&emptyPos);
            }
        }
    }

    template <class T>
    void WritePointers(std::map<int, TPZAutoPointer<T>> &vec) {
        int nc = vec.size(), emptyPos = -1;
        this->Write(&nc);
        typedef typename std::map<int, TPZAutoPointer<T>>::iterator vec_it;
        vec_it it;
        for (it = vec.begin(); it != vec.end(); it++) {
            int id = it->first;
            this->Write(&id);
            if (it->second) {
                it->second->Write(*this, 1);
            } else {
                this->Write(&emptyPos);
            }
        }
    }

    template <class T> void WritePointers(std::map<int, T *> &vec) {
        int nc = vec.size(), emptyPos = -1;
        this->Write(&nc);
        typedef typename std::map<int, T *>::iterator vec_it;
        vec_it it;
        for (it = vec.begin(); it != vec.end(); it++) {
            int id = it->first;
            this->Write(&id);
            if (it->second) {
                it->second->Write(*this, 1);
            } else {
                this->Write(&emptyPos);
            }
        }
    }
    template <class T> void WritePointers(std::set<T *> &vec) {
        int nel = vec.size();
        this->Write(&nel);
        typedef typename std::set<T *>::iterator vec_it;
        vec_it it;
        while (it != vec.end()) {
            it->Write(*this, 1);
            it++;
        }
    }

    template <class T, int EXP>
    void WritePointers(TPZChunkVector<T *, EXP> &vec) {
        long c, nc = vec.NElements();
        int emptyPos = -1;
        this->Write(&nc);
        for (c = 0; c < nc; c++) {
            T *ptr = vec[c];
            if (ptr)
                ptr->Write(*this);
            else
                this->Write(&emptyPos);
        }
    }

    template <class T, int EXP>
    void WritePointers(TPZAdmChunkVector<T *, EXP> &vec) {
        long c, nc = vec.NElements();
        int emptyPos = -1;
        this->Write(&nc);
        for (c = 0; c < nc; c++) {
            T *ptr = vec[c];
            if (ptr)
                ptr->Write(*this, 1);
            else
                this->Write(&emptyPos);
        }
        this->Write(&vec.fCompactScheme);
        Write(vec.fFree);
        Write(vec.fNFree);
    }

    /**
     * @brief Methods to read objects or pointer for objects.
     */

    template <class T> void Read(std::vector<T> &vec, void *context) {
        int c, nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c].Read(*this, context);
        }
    }

    template <class T> void Read(TPZVec<T> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c].Read(*this, context);
        }
    }

    template <class T> void Read(TPZVec<T> &vec){
    	Read(vec,NULL);
    }

	template <int N> void Read(TPZManVector<REAL, N> &vec){
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

    template <class T, int EXP>
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

    template <class T> void Read(std::map<T, T> &vec) {
        TPZManVector<T> cp;
        this->Read(cp);
        int sz = cp.NElements();
        int i;
        for (i = 0; i < sz; i += 2) {
            vec[cp[i]] = cp[i + 1];
        }
    }

    void Read(std::string &vec);

    void Read(TPZVec<std::string> &vec);

    template <class T> void Read(std::set<T> &vec) {
        int nel;
        this->Read(&nel, 1);
        for (int i = 0; i < nel; i++) {
            T val;
            this->Read(&val);
            vec.insert(val);
        }
    }
    
    template <class T> void ReadPointers(TPZVec<T *> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c] = dynamic_cast<T *>(TPZSaveable::Restore(*this, context));
        }
    }
    
    template <class T>
    void ReadPointers(std::map<int, TPZAutoPointer<T>> &vec,
                                 void *context) {
        int c, nc;
        this->Read(&nc, 1);
        for (c = 0; c < nc; c++) {
            int id;
            this->Read(&id, 1);
            vec[id] =
            TPZAutoPointer<T>(dynamic_cast<T *>(TPZSaveable::Restore(*this, context)));
        }
    }
    
    template <class T>
    void ReadPointers(std::map<int, T *> &vec, void *context) {
        int c, nc;
        this->Read(&nc, 1);
        for (c = 0; c < nc; c++) {
            int id;
            this->Read(&id, 1);
            vec[id] = (dynamic_cast<T *>(TPZSaveable::Restore(*this, context)));
        }
    }
    
    template <class T, int EXP>
    void ReadPointers(TPZChunkVector<T *, EXP> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c] = dynamic_cast<T *>(TPZSaveable::Restore(*this, context));
        }
    }
    
    template <class T, int EXP>
    void ReadPointers(TPZAdmChunkVector<T *, EXP> &vec, void *context) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) {
            vec[c] = dynamic_cast<T *>(TPZSaveable::Restore(*this, context));
        }
        this->Read(&vec.fCompactScheme, 1);
        Read(vec.fFree);
        Read(vec.fNFree);
    }
    
    ///////TEMPLATE SPECIALIZATIONS
      void Write(const TPZVec<float> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<double> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
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
    
      void Write(const TPZVec<std::complex<double> > &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<long double> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<std::complex<long double> > &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<std::complex<float> > &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<int> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
      void Write(const TPZVec<long> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<char> &vec) {
        long nel = vec.NElements();
        this->Write(&nel);
        if (nel) this->Write(&vec[0], vec.NElements());
      }
    
      void Write(const TPZVec<std::string> &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        for (int i = 0; i < nel; i++) {
          Write(vec[i]);
        }
      }
    
      void Write(const std::set<int> &vec) {
        int nel = ((int)vec.size());
        this->Write(&nel);
        std::set<int>::iterator it = vec.begin();
        while (it != vec.end()) {
          int val = *it;
          this->Write(&val);
          it++;
        }
      }
    
      void Write(const std::vector<float> &vec) {
        int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<double> &vec) {
        int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<std::complex<double> > &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<long double> &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<std::complex<long double> > &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<std::complex<float> > &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<TPZFlopCounter> &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<int> &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::vector<char> &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
    
      void Write(const std::string &vec) {
          int nel = ((int)vec.size());
        this->Write(&nel);
        if (nel) this->Write(&vec[0], ((int)vec.size()));
      }
     
     
     
     
       void Read(std::set<int> &vec) {
        int nel;
        this->Read(&nel, 1);
        for (int i = 0; i < nel; i++) {
          int val;
          this->Read(&val);
          vec.insert(val);
        }
      }
    
      void Read(TPZVec<int> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
      void Read(TPZVec<long> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
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
    
      void Read(std::vector<int> &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
      void Read(std::vector<long> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(std::vector<float> &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<float> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(std::vector<double> &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<double> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(std::vector<long double> &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<long double> &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      template <class T, int EXP>
      void Read(TPZAdmChunkVector<T, EXP> &vec) {
        long c, nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        for (c = 0; c < nc; c++) this->Read(&vec[c], 1);
        this->Read(&vec.fCompactScheme, 1);
        Read(*this, vec.fFree);
        Read(*this, vec.fNFree);
      }
    
      void Read(std::vector<std::complex<float> > &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<std::complex<float> > &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(std::vector<std::complex<double> > &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<std::complex<double> > &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(std::vector<std::complex<long double> > &vec) {
        int nc;
        this->Read(&nc, 1);
        vec.resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
    
      void Read(TPZVec<std::complex<long double> > &vec) {
        long nc;
        this->Read(&nc, 1);
        vec.Resize(nc);
        if (nc) this->Read(&vec[0], nc);
      }
};

/**
 * @ingroup save
 * @brief Implements reading from and writing to an ascii file. \ref save "Persistency"
 */
class TPZFileStream : public TPZStream {
	
	std::ofstream fo;
	std::ifstream fi;
	
public:
	
	TPZFileStream() { }
	
	virtual ~TPZFileStream() { }
	
	void OpenWrite(const std::string &filename) {
		fo.open(filename.c_str());
        fo.precision(15);
	}
	
	void OpenRead(const std::string &filename) {
		fi.open(filename.c_str());
	}
    bool eof() {
        return fi.eof();
    }
	
	virtual void Write(const int *p, int size) {
		Writes<int>(p,size);
	}
	
	virtual void Write(const unsigned int *p, int size) {
		Writes<unsigned int>(p,size);
	}
	
	virtual void Write(const long *p, int size) {
		Writes<long>(p,size);
	}

	virtual void Write(const float *p, int size) {
		Writes<float>(p,size);
	}
	
	virtual void Write(const double *p, int size) {
		Writes<double>(p,size);
	}
	
	virtual void Write(const long double *p, int size) {
		Writes<long double>(p,size);
	}
	
	virtual void Write(const char *p, int size) {
		Writes<char>(p,size);
	}
	
	virtual void Write(const std::string *p, int size) {
		Writes<std::string>(p,size);
	}

	virtual void Write(const std::complex <float> *p, int size) {
		Writes< std::complex <float> >(p,size);
	}

	virtual void Write(const std::complex <double> *p, int size) {
		Writes< std::complex <double> >(p,size);
	}

	virtual void Write(const std::complex <long double> *p, int size) {
		Writes< std::complex <long double> >(p,size);
	}
    
#ifdef _AUTODIFF
    
    virtual void Write(const Fad <float> *p, int size) {
        Writes< Fad <float> >(p,size);
    }
    
    virtual void Write(const Fad <double> *p, int size) {
        Writes< Fad <double> >(p,size);
    }
    
    virtual void Write(const Fad <long double> *p, int size) {
        Writes< Fad <long double> >(p,size);
    }
    
#endif

	template<class T>
	void  Writes(const T *p, int size) 
	{
		int c;
		for(c=0; c<size; c++) fo << p[c] << std::endl;
	}
	
	virtual void Read(int *p, int size) {
		Reads<int>(p,size);
	}
	
	virtual void Read(unsigned int *p, int size) {
		Reads<unsigned int>(p,size);
	}
	
	virtual void Read(long *p, int size) {
		Reads<long>(p,size);
	}
	
	virtual void Read(float *p, int size) {
		Reads<float>(p,size);
	}
	
	virtual void Read(double *p, int size) {
		Reads<double>(p,size);
	}
	
	virtual void Read(long double *p, int size) {
		Reads<long double>(p,size);
	}
	
	virtual void Read(char *p, int size) {
		Reads<char>(p,size);
	}
	
	virtual void Read(std::string *p, int size) {
		int c;
		char buf[2560];
		for(c=0; c<size; c++) 
		{
			fi.getline(buf,2560);
			p[c] = buf;
		}
	}

	virtual void Read(std::complex <float> *p, int size) {
		Reads< std::complex <float> >(p,size);
	}

	virtual void Read(std::complex <double> *p, int size) {
		Reads< std::complex <double> >(p,size);
	}

	virtual void Read(std::complex <long double> *p, int size) {
		Reads< std::complex <long double> >(p,size);
	}
    
#ifdef _AUTODIFF
    
    virtual void Read(Fad <float> *p, int size) {
        Reads< Fad <float> >(p,size);
    }
    
    virtual void Read(Fad <double> *p, int size) {
        Reads< Fad <double> >(p,size);
    }
    
    virtual void Read(Fad <long double> *p, int size) {
        Reads< Fad <long double> >(p,size);
    }
    
#endif

	template<class T>
	void Reads(T *p, int size) {
		int c;
		char buf[100];
        if(!fi)
        {
            DebugStop();
        }
		if(size)
		{
			for(c=0; c<size; c++) fi >> p[c];
			fi.getline(buf,100);
		}
	}
};

#endif
