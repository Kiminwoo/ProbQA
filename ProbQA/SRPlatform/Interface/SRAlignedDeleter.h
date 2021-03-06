// Probabilistic Question-Answering system
// @2017 Sarge Rogatch
// This software is distributed under GNU AGPLv3 license. See file LICENSE in repository root for details.

#pragma once

namespace SRPlat {

class SRAlignedDeleter {
public:
  void operator()(void *p) { _mm_free(p); }
};

template<typename T> using AlignedUniquePtr = std::unique_ptr<T, SRAlignedDeleter>;

} // namespace SRPlat
