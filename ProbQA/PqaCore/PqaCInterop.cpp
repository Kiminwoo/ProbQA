// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#include "stdafx.h"
#include "../PqaCore/Interface/PqaCInterop.h"
#include "../PqaCore/Interface/IPqaEngineFactory.h"
#include "../PqaCore/Interface/IPqaEngine.h"

//TODO: catch exceptions
//TODO: handle null pointers
//TODO: return errors of the current file's logic

using namespace ProbQA;
using namespace SRPlat;

// Otherwise we have to do translations, placing AnsweredQuestion objects on the stack
static_assert(sizeof(CiAnsweredQuestion) == sizeof(AnsweredQuestion)
  && offsetof(CiAnsweredQuestion, _iQuestion) == offsetof(AnsweredQuestion, _iQuestion)
  && offsetof(CiAnsweredQuestion, _iAnswer) == offsetof(AnsweredQuestion, _iAnswer));

static_assert(sizeof(CiRatedTarget) == sizeof(RatedTarget)
  && offsetof(CiRatedTarget, _iTarget) == offsetof(RatedTarget, _iTarget)
  && offsetof(CiRatedTarget, _prob) == offsetof(RatedTarget, _prob));

namespace {

char *PrepareSRString(const SRString &s) {
  const char *pSrc;
  size_t len = s.GetData(pSrc);
  char *pDest = new char[len + 1];
  memcpy(pDest, pSrc, len);
  pDest[len] = 0;
  return pDest;
}

void SendUnexpectedError(void **ppStrErr) {
  const char srcMsg[] = "Unexpected error.";
  constexpr size_t len = sizeof(srcMsg);
  char *pMsg = new char[len];
  memcpy(pMsg, srcMsg, len);
  *ppStrErr = pMsg;
}

void AssignPqaError(void **ppError, PqaError &err) {
  if (err.IsOk()) {
    *ppError = nullptr;
  }
  else
  {
    PqaError *pErr = new PqaError(std::move(err));
    *ppError = pErr;
  }
}

void* ReturnPqaError(PqaError &&err) {
  if (err.IsOk()) {
    return nullptr;
  }
  return new PqaError(std::move(err));
}

} // anonymous namespace

PQACORE_API void* CiGetPqaEngineFactory() {
  return &(PqaGetEngineFactory());
}

PQACORE_API void* PqaEngineFactory_CreateCpuEngine(void* pvFactory, void **ppError, const CiEngineDefinition *pEngDef) {
  IPqaEngineFactory *pEf = static_cast<IPqaEngineFactory *>(pvFactory);
  if (pEf == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngineFactory."));
    return nullptr;
  }
  EngineDefinition engDef;
  engDef._dims._nAnswers = pEngDef->_nAnswers;
  engDef._dims._nQuestions = pEngDef->_nQuestions;
  engDef._dims._nTargets = pEngDef->_nTargets;
  engDef._prec._type = static_cast<TPqaPrecisionType>(pEngDef->_precType);
  engDef._prec._mantissa = pEngDef->_precMantissa;
  engDef._prec._exponent = pEngDef->_precExponent;
  engDef._initAmount = pEngDef->_initAmount;
  engDef._memPoolMaxBytes = pEngDef->_memPoolMaxBytes;
  PqaError err;
  IPqaEngine *pEngine = pEf->CreateCpuEngine(err, engDef);
  AssignPqaError(ppError, err);
  return pEngine;
}

PQACORE_API void* PqaEngineFactory_LoadCpuEngine(void *pvFactory, void **ppError, const char* filePath,
  uint64_t memPoolMaxBytes)
{
  IPqaEngineFactory *pEf = static_cast<IPqaEngineFactory *>(pvFactory);
  if (pEf == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngineFactory."));
    return nullptr;
  }
  PqaError err;
  IPqaEngine *pEngine = pEf->LoadCpuEngine(err, filePath, memPoolMaxBytes);
  AssignPqaError(ppError, err);
  return pEngine;
}

PQACORE_API void CiReleasePqaError(void *pvErr) {
  PqaError *pPe = static_cast<PqaError*>(pvErr);
  delete pPe;
}

PQACORE_API void CiReleaseString(void *pvString) {
  char *pS = static_cast<char*>(pvString);
  delete pS;
}

PQACORE_API void* PqaError_ToString(void *pvError, const uint8_t withParams) {
  PqaError *pErr = static_cast<PqaError*>(pvError);
  SRString srStr = pErr->ToString(withParams);
  return PrepareSRString(srStr);
}

PQACORE_API uint8_t Logger_Init(void **ppStrErr, const char* baseName) {
  try {
    SRDefaultLogger::Init(SRString::MakeUnowned(baseName));
    *ppStrErr = nullptr;
    return 1;
  }
  catch (const std::exception& ex) {
    size_t len = strlen(ex.what());
    char *pMsg = new char[len+1];
    memcpy(pMsg, ex.what(), len + 1);
    *ppStrErr = pMsg;
    return 0;
  }
  catch (const SRException& ex) {
    SRString strEx = ex.ToString();
    const char* pS;
    const size_t len = strEx.GetData(pS);
    char *pMsg = new char[len + 1];
    memcpy(pMsg, pS, len);
    pMsg[len] = 0;
    *ppStrErr = pMsg;
    return 0;
  }
  catch (...) {
    SendUnexpectedError(ppStrErr);
    return 0;
  }
}

PQACORE_API void CiReleasePqaEngine(void *pvEngine) {
  IPqaEngine *pPe = static_cast<IPqaEngine*>(pvEngine);
  delete pPe;
}

PQACORE_API void* PqaEngine_Train(void *pvEngine, int64_t nQuestions, const CiAnsweredQuestion* const pAQs,
  const int64_t iTarget, const double amount)
{
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    return new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
  }
  return ReturnPqaError(pEng->Train(nQuestions, reinterpret_cast<const AnsweredQuestion*>(pAQs), iTarget, amount));
}

PQACORE_API uint8_t PqaEngine_QuestionPermFromComp(void *pvEngine, const int64_t count, int64_t *pIds) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    SRDefaultLogger::Get()->Log(ISRLogger::Severity::Error, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  return pEng->QuestionPermFromComp(count, pIds) ? 1 : 0;
}

PQACORE_API uint8_t PqaEngine_QuestionCompFromPerm(void *pvEngine, const int64_t count, int64_t *pIds) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    SRDefaultLogger::Get()->Log(ISRLogger::Severity::Error, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  return pEng->QuestionCompFromPerm(count, pIds) ? 1 : 0;
}

PQACORE_API uint8_t PqaEngine_TargetPermFromComp(void *pvEngine, const int64_t count, int64_t *pIds) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    SRDefaultLogger::Get()->Log(ISRLogger::Severity::Error, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  return pEng->TargetPermFromComp(count, pIds) ? 1 : 0;
}

PQACORE_API uint8_t PqaEngine_TargetCompFromPerm(void *pvEngine, const int64_t count, int64_t *pIds) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    SRDefaultLogger::Get()->Log(ISRLogger::Severity::Error, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  return pEng->TargetCompFromPerm(count, pIds) ? 1 : 0;
}

PQACORE_API uint64_t PqaEngine_GetTotalQuestionsAsked(void *pvEngine, void **ppError) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  PqaError err;
  const uint64_t nQAs = pEng->GetTotalQuestionsAsked(err);
  AssignPqaError(ppError, err);
  return nQAs;
}

PQACORE_API uint8_t PqaEngine_CopyDims(void *pvEngine, CiEngineDimensions *pDims) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    SRDefaultLogger::Get()->Log(ISRLogger::Severity::Error, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return 0;
  }
  const EngineDimensions dims = pEng->CopyDims();
  pDims->_nAnswers = dims._nAnswers;
  pDims->_nQuestions = dims._nQuestions;
  pDims->_nTargets = dims._nTargets;
  return 1;
}

PQACORE_API int64_t PqaEngine_StartQuiz(void *pvEngine, void **ppError) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return cInvalidPqaId;
  }
  PqaError err;
  const TPqaId iQuiz = pEng->StartQuiz(err);
  AssignPqaError(ppError, err);
  return iQuiz;
}

PQACORE_API int64_t PqaEngine_ResumeQuiz(void *pvEngine, void **ppError, const int64_t nAnswered,
  const CiAnsweredQuestion* const pAQs)
{
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return cInvalidPqaId;
  }
  PqaError err;
  const TPqaId iQuiz = pEng->ResumeQuiz(err, nAnswered, reinterpret_cast<const AnsweredQuestion*>(pAQs));
  AssignPqaError(ppError, err);
  return iQuiz;
}

PQACORE_API int64_t PqaEngine_NextQuestion(void *pvEngine, void **ppError, const int64_t iQuiz) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return cInvalidPqaId;
  }
  PqaError err;
  const TPqaId iQuestion = pEng->NextQuestion(err, iQuiz);
  AssignPqaError(ppError, err);
  return iQuestion;
}

PQACORE_API void* PqaEngine_RecordAnswer(void *pvEngine, const int64_t iQuiz, const int64_t iAnswer) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    return new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
  }
  return ReturnPqaError(pEng->RecordAnswer(iQuiz, iAnswer));
}

PQACORE_API int64_t PqaEngine_ListTopTargets(void *pvEngine, void **ppError, const int64_t iQuiz,
  const int64_t maxCount, CiRatedTarget *pDest)
{
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return cInvalidPqaId;
  }
  PqaError err;
  const TPqaId nListed = pEng->ListTopTargets(err, iQuiz, maxCount, reinterpret_cast<RatedTarget*>(pDest));
  AssignPqaError(ppError, err);
  return nListed;
}

PQACORE_API void* PqaEngine_RecordQuizTarget(void *pvEngine, const int64_t iQuiz, const int64_t iTarget,
  const double amount)
{
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    return new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
  }
  return ReturnPqaError(pEng->RecordQuizTarget(iQuiz, iTarget, amount));
}

PQACORE_API void* PqaEngine_ReleaseQuiz(void *pvEngine, const int64_t iQuiz) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    return new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
  }
  return ReturnPqaError(pEng->ReleaseQuiz(iQuiz));
}

PQACORE_API void* PqaEngine_SaveKB(void *pvEngine, const char* const filePath, const uint8_t bDoubleBuffer) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    return new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
  }
  return ReturnPqaError(pEng->SaveKB(filePath, bDoubleBuffer != 0));
}

PQACORE_API int64_t PqaEngine_GetActiveQuestionId(void *pvEngine, void **ppError, const int64_t iQuiz) {
  IPqaEngine *pEng = static_cast<IPqaEngine*>(pvEngine);
  if (pEng == nullptr) {
    *ppError = new PqaError(PqaErrorCode::NullArgument, nullptr, SRString::MakeUnowned(
      SR_FILE_LINE "Nullptr is passed in place of IPqaEngine."));
    return cInvalidPqaId;
  }
  PqaError err;
  int64_t iQuestion = pEng->GetActiveQuestionId(err, iQuiz);
  AssignPqaError(ppError, err);
  return iQuestion;
}
