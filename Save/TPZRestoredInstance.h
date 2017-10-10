#ifndef TPZRESTOREOBJ_H
#define TPZRESTOREOBJ_H

#include <ostream>       // for operator<<
#include "pzmanvector.h" // for TPZManVector
#include "pzvec.h" // for TPZVec
#include "tpzautopointer.h"
#include "TPZSavable.h"

class TPZSavable;
class TPZContBufferedStream;

class TPZRestoredInstance {
  public:
    TPZRestoredInstance();
    TPZRestoredInstance(TPZSavable *);
    void SetInstance(TPZSavable *);
    TPZSavable *GetPointerToMyObj() const;
    TPZAutoPointer<TPZSavable> GetAutoPointerToMyObj();
    TPZVec<int> &MyPointersVec();
    void SetObjId(const long unsigned int &objId);
    long unsigned int GetObjId() const;
    void SetClassId(const int &classId);
    int GetClassId() const;
    virtual ~TPZRestoredInstance();
  protected:
    TPZSavable *mpInstance;
    TPZManVector<int, 1> mPointersVec;
    TPZAutoPointer<TPZSavable> mAutoPointerToInstance;
};

#endif // TPZRESTOREOBJ_H