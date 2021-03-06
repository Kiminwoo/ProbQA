// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

#include "../PqaCore/CENormPriorsTask.fwd.h"
#include "../PqaCore/CEQuiz.fwd.h"
#include "../PqaCore/CpuEngine.fwd.h"
#include "../PqaCore/CEBaseTask.h"

namespace ProbQA {

#pragma warning( push )
#pragma warning( disable : 4324 ) // structure was padded due to alignment specifier
template<typename taNumber> class CENormPriorsTask : public CEBaseTask {
public: // types
  typedef taNumber TNumber;

private: // variables
  const CEQuiz<taNumber> *const _pQuiz;

public:
  // The number to add to the exponent so to get it within the representable range or to cut off if corrected exponent
  //   is too small. Repeated in each 64-bit component.
  __m256i _corrExp;
  SRPlat::SRNumPack<taNumber> _sumPriors;

public:
  explicit inline CENormPriorsTask(CpuEngine<taNumber> &engine, CEQuiz<taNumber> &quiz);

  const CEQuiz<taNumber>& GetQuiz() const { return *_pQuiz; }
};
#pragma warning( pop )

template<typename taNumber> inline CENormPriorsTask<taNumber>::CENormPriorsTask(CpuEngine<taNumber> &engine,
  CEQuiz<taNumber> &quiz) : CEBaseTask(engine), _pQuiz(&quiz)
{ }

} // namespace ProbQA
