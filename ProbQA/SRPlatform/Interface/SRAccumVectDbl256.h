// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

#include "../SRPlatform/Interface/SRSimd.h"
#include "../SRPlatform/Interface/SRMacros.h"

namespace SRPlat {

// 256-bit vector of Kahan summators for double precision scalars.
class SRAccumVectDbl256 {
  __m256d _sum;
  __m256d _corr;

public:
  explicit SRAccumVectDbl256() : _sum(_mm256_setzero_pd()), _corr(_mm256_setzero_pd()) {}
  void Reset() {
    _sum = _mm256_setzero_pd();
    _corr = _mm256_setzero_pd();
  }
  inline SRAccumVectDbl256& __vectorcall Add(const __m256d value);
  inline SRAccumVectDbl256& __vectorcall Add(SRVectCompCount at, const double value);
  //Note: this method is not at maximum precision.
  inline double __vectorcall GetFullSum();
  //TODO: sum all components in this and fellow separately in AVX2, return the sum of this, and set fellowSum to the
  //  sum of fellow.
  inline double __vectorcall PairSum(const SRAccumVectDbl256& fellow, double& fellowSum);
};

FLOAT_PRECISE_BEGIN
inline SRAccumVectDbl256& __vectorcall SRAccumVectDbl256::Add(const __m256d value) {
  const __m256d y = _mm256_sub_pd(value, _corr);
  const __m256d t = _mm256_add_pd(_sum, y);
  _corr = _mm256_sub_pd(_mm256_sub_pd(t, _sum), y);
  _sum = t;
  return *this;
}
inline SRAccumVectDbl256& __vectorcall SRAccumVectDbl256::Add(SRVectCompCount at, const double value) {
  const double y = value - _corr.m256d_f64[at];
  const double t = _sum.m256d_f64[at] + y;
  _corr.m256d_f64[at] = (t - _sum.m256d_f64[at]) - y;
  _sum.m256d_f64[at] = t;
  return *this;
}
inline double __vectorcall SRAccumVectDbl256::GetFullSum() {
  const __m256d interleaved = _mm256_hadd_pd(_corr, _sum);
  const __m128d corrSum = _mm_add_pd(_mm256_extractf128_pd(interleaved, 1), _mm256_castpd256_pd128(interleaved));
  return corrSum.m128d_f64[1] - corrSum.m128d_f64[0];
}
FLOAT_PRECISE_END

} // namespace SRPlat
