#ifndef TPZRESTOREOBJ_H
#define TPZRESTOREOBJ_H

#include <ostream>       // for operator<<
#include "pzmanvector.h" // for TPZManVector
class TPZSaveable;
template <class T> class TPZVec;

class TPZRestoredInstance {
  public:
    TPZRestoredInstance();
    TPZRestoredInstance(TPZSaveable *);
    void SetPtrsAsRestored();
    bool HaveMyPtrsBeenAssigned() const;
    void SetInstance(TPZSaveable *);
    TPZSaveable *GetPointerToMyObj() const;
    TPZVec<int> &MyPointersVec();

  protected:
    TPZSaveable *mpInstance;
    bool mAssignedPointers;
    TPZManVector<int, 1> mPointersVec;
};

#endif // TPZRESTOREOBJ_H