#include "TPZPersistenceManager.h"
#include <stddef.h>            // for NULL
#include "TPZBFileStream.h"    // for TPZBFileStream
#include "TPZFileStream.h"     // for TPZFileStream
#include "TPZGeneralFStream.h" // for TPZGeneralFStream
#include "TPZSavable.h"       // for TPZSavable
#include "pzerror.h"           // for DebugStop, PZError
#include "pzvec.h"
#include "TPZContBufferedStream.h"             // for TPZVec
#include "TPZChunkInTranslation.h"

using namespace TPZPersistenceManagerNS;

TPZGeneralFStream *TPZPersistenceManager::mpStream;

// for READING from file
TPZVec<TPZRestoredInstance> TPZPersistenceManager::mObjVec;
TPZVec<TPZAutoPointer<TPZChunkInTranslation>> TPZPersistenceManager::mChunksVec;
TPZVec<long int> TPZPersistenceManager::mMainObjIds;
unsigned int TPZPersistenceManager::mNextMainObjIndex;

// for WRITING to file
std::map<std::string, long unsigned int> TPZPersistenceManager::mFileVersionInfo;
TPZContBufferedStream TPZPersistenceManager::mObjectsStream;
TPZVec<const TPZSavable *> TPZPersistenceManager::mPointersToSave;
TPZContBufferedStream TPZPersistenceManager::mCurrentObjectStream;
std::map<const TPZSavable *, long int> TPZPersistenceManager::mObjMap;
long int TPZPersistenceManager::mNextPointerToSave;

TPZPersistenceManager::TPZPersistenceManager() {
    mpStream = NULL;
    mObjVec.Resize(0);
    mObjMap.clear();
}

/********************************************************************
 *                                                                   *
 *                   WRITE-RELATED METHODS                           *
 *                                                                   *
 ********************************************************************/
void TPZPersistenceManager::OpenWrite(const std::string &fileName,
        const streamType cStreamType) {

    switch (cStreamType) {
        case binary:
        {
            mpStream = new TPZBFileStream();
            break;
        }
        case ascii:
        {
            mpStream = new TPZFileStream();
            break;
        }
        default:
            DebugStop();
    }

    mpStream->OpenWrite(fileName);

    std::string versionString("FileVersion");
    mpStream->Write(versionString.c_str(),11);
    
#ifdef PZDEBUG
    if (!mpStream->AmIOpenForWrite()) {
        DebugStop();
    }
    if (mPointersToSave.size() > 0) {
        DebugStop();
    }
#endif    
    mFileVersionInfo.clear();
    mObjectsStream.clear();
    mObjMap.clear();
    mPointersToSave.clear();
    mMainObjIds.clear();
    mNextPointerToSave = 0;
}

void TPZPersistenceManager::WriteToFile(const TPZSavable *obj) {
    auto objId = ScheduleToWrite(obj);
    unsigned int nMainObjIds = mMainObjIds.size();
    mMainObjIds.resize(nMainObjIds + 1);
    mMainObjIds[nMainObjIds] = objId;

#ifdef PZDEBUG    
    unsigned long written = 0x46;
#endif
    for (; mNextPointerToSave < mPointersToSave.size(); ++mNextPointerToSave) {
        // writes obj-id
        mObjectsStream.Write(&mNextPointerToSave, 1);
        auto pointer = mPointersToSave[mNextPointerToSave];
        // writes classId
        auto classId = pointer->ClassId();
        if (classId == -1 || classId == 666){
            DebugStop();
        }
        mObjectsStream.Write(&classId, 1);
        // writes object data
        mCurrentObjectStream.clear();
        pointer->Write(mCurrentObjectStream, 0);
        unsigned int size = mCurrentObjectStream.Size();
        mObjectsStream.Write(&size, 1);
        mObjectsStream << mCurrentObjectStream;
#ifdef PZDEBUG    
        std::cout << "ObjId: " << mNextPointerToSave 
                  << " ClassId: " << classId 
                  << " range: " << written+1 << " (0x" << std::hex << written+1 << std::dec <<  ") - " 
                << written + sizeof(long int) + 2*sizeof(int) + size 
                << "(0x" << std::hex << written + sizeof(long int) + 2*sizeof(int) + size << ")" << std::dec 
                  << " size: " << size << std::endl;
        written += sizeof(long int) + 2*sizeof(int) + size;
#endif
    }
}

void TPZPersistenceManager::CloseWrite() {
    mpStream->Write(mFileVersionInfo);

    const long unsigned int nObjects = mPointersToSave.size();
    mpStream->Write(&nObjects);
    
    size_t nObjectBytes = mObjectsStream.Size();
    char *temp = new char[nObjectBytes];
    mObjectsStream.GetDataFromBuffer(temp);
    mpStream->Write(temp, nObjectBytes);
	delete[] temp;

    unsigned int nMainObjects = mMainObjIds.size();
    mpStream->Write(&nMainObjects);
    for (auto mainObjId : mMainObjIds) {
        mpStream->Write(&mainObjId);
    }

    mpStream->CloseWrite();
    mFileVersionInfo.clear();
    mObjectsStream.clear();
    mObjMap.clear();
    mPointersToSave.clear();
    mMainObjIds.clear();
    mNextPointerToSave = 0;
}

long int TPZPersistenceManager::ScheduleToWrite(const TPZSavable *obj) {
    if (!obj) return -1;
    auto iMap = mObjMap.find(obj);
    long int objId;
    if (iMap == mObjMap.end()) { //object isn't in the map yet
        objId = mObjMap.size();
        if (mPointersToSave.size() != objId) {
            DebugStop();
        }
        mObjMap.insert(std::make_pair(obj, objId));
        mFileVersionInfo.insert(obj->Version());
        mPointersToSave.resize(objId + 1);
        mPointersToSave[objId] = obj;
    } else {
        objId = iMap->second;
    }
    return objId;
}

void TPZPersistenceManager::WritePointer(const TPZSavable *obj, TPZStream *stream) {
    const long int objId = ScheduleToWrite(obj);
    stream->Write(&objId);
}

/********************************************************************
 *                                                                   *
 *                    READ-RELATED METHODS                           *
 *                                                                   *
 ********************************************************************/

unsigned int TPZPersistenceManager::OpenRead(const std::string &fileName,
        const streamType cStreamType) {

    if (TPZSavable::ClassIdMap().size() == 0){
        for (const auto &restoreClass : TPZSavable::RestoreClassSet()) {
            TPZSavable::RegisterClassId(restoreClass->Restore()->ClassId(), restoreClass);
        }
    }

    switch (cStreamType) {
        case binary:
        {
            mpStream = new TPZBFileStream();
            break;
        }
        case ascii:
        {
            mpStream = new TPZFileStream();
            break;
        }
        default:
            DebugStop();
    }
    mpStream->OpenRead(fileName);
    std::string versionString("FileVersion");
    char versionRead[12];
    mpStream->Read(versionRead, 11);
    versionRead[11] = '\0'; // terminates c-style string

    if (versionString.compare(versionRead) != 0) { // unversioned file --- not readable
        PZError << "Error reading file - unversioned file" << std::endl;
        DebugStop();
    }

    mFileVersionInfo.clear();
    mpStream->Read(mFileVersionInfo);

    long unsigned int nObjects;
    mpStream->Read(&nObjects);
    mChunksVec.Resize(nObjects);

    long int objId;
    int classId;
    unsigned int objSize;
    for (long unsigned int i = 0; i < nObjects; i++) {
        mpStream->Read(&objId);
        mpStream->Read(&classId);
        mpStream->Read(&objSize);
        mChunksVec[objId] = new TPZChunkInTranslation(objId, classId, *mpStream, objSize, mFileVersionInfo);
    }

    //    std::map<std::string, long unsigned int> currentVersionInfo = mFileVersionInfo;
    //    std::map<std::string, long unsigned int> nextVersion = ComputeNextVersion(currentVersionInfo);
    //    while (){ //nextVersion is not null
    //        // move newChunk to oldChunk
    //        for (long unsigned int i = 0; i < nObjects; i++) {
    //            int classId = mChunksVec[i]->GetClassId();
    //            if (classId != -1) {
    //                //translate
    //            }
    //        }
    //        currentVersionInfo = nextVersion;
    //        nextVersion = ComputeNextVersion(currentVersionInfo);
    //    }
    // allocate
    mObjVec.Resize(mChunksVec.size());
    for (unsigned int i = 0; i < mObjVec.size(); ++i) {
        int classId = mChunksVec[i]->GetClassId();
        if (classId != -1) {
            mObjVec[i].ResetReadStatus();
            mObjVec[i].SetInstance(TPZSavable::CreateInstance(classId));
        }
    }
    
    for (unsigned int i = 0; i < mObjVec.size(); ++i) {
        int classId = mChunksVec[i]->GetClassId();
        if (classId != -1 && !mObjVec[i].IsAlreadyRead()) {
            mObjVec[i].SetRead();
            mObjVec[i].GetPointerToMyObj()->Read(mChunksVec[i]->mNewStream, NULL );
#ifdef PZDEBUG    
            if (mChunksVec[i]->mNewStream.Size() != 0){
                DebugStop();
            }
#endif
        }
    }

    unsigned int nMainObjects;
    mpStream->Read(&nMainObjects);
    mMainObjIds.resize(nMainObjects);
	for (unsigned int i = 0; i < nMainObjects; ++i) {
        mpStream->Read(&mMainObjIds[i]);
    }
    mpStream->CloseRead();

    mNextMainObjIndex = 0;
    return nMainObjects;
}

TPZRestoredInstance *TPZPersistenceManager::NewRestoredInstance() {
    auto nObjects = mObjVec.size();
    mObjVec.Resize(nObjects + 1);
    return &mObjVec[nObjects];
}

TPZSavable *TPZPersistenceManager::ReadFromFile() {
    TPZSavable *obj = GetInstance(mMainObjIds[mNextMainObjIndex++]);
    if (mNextMainObjIndex == mMainObjIds.size()){
        CloseRead();
    }
    return obj;
}

void TPZPersistenceManager::AddInstanceToVec(TPZSavable *obj, const int &cId) {
    mObjVec[cId].SetInstance(obj);
}

TPZSavable *TPZPersistenceManager::GetInstance(const long int &objId) {
    if (objId != -1){
        if (!mObjVec[objId].IsAlreadyRead()){            
            mObjVec[objId].SetRead();
            mObjVec[objId].GetPointerToMyObj()->Read(mChunksVec[objId]->mNewStream, NULL );
            if (mChunksVec[objId]->mNewStream.Size() != 0){
                DebugStop();
            }
        }
        return mObjVec[objId].GetPointerToMyObj();
    }
    return NULL;
}

TPZSavable *TPZPersistenceManager::GetInstance(TPZStream *stream) {
    long int objId;
    stream->Read(&objId);
    return GetInstance(objId);
}

TPZAutoPointer<TPZSavable> TPZPersistenceManager::GetAutoPointer(const long int &objId) {
    TPZAutoPointer<TPZSavable> autoPointer;
    if (objId != -1) {
        if (!mObjVec[objId].IsAlreadyRead()){            
            mObjVec[objId].SetRead();
            mObjVec[objId].GetPointerToMyObj()->Read(mChunksVec[objId]->mNewStream, NULL );
            if (mChunksVec[objId]->mNewStream.Size() != 0){
                DebugStop();
            }
        }
        return mObjVec[objId].GetAutoPointerToMyObj();
    }
    return autoPointer;
}

TPZAutoPointer<TPZSavable> TPZPersistenceManager::GetAutoPointer(TPZStream *stream) {
    long int objId;
    stream->Read(&objId);
    return GetAutoPointer(objId);
}

void TPZPersistenceManager::CloseRead() {
    mMainObjIds.clear();
    mChunksVec.clear();
    mObjVec.clear();
    mNextMainObjIndex = 0;
}
