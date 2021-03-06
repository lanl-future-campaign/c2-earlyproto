/*
 * Copyright (c) 2021 Triad National Security, LLC, as operator of Los Alamos
 * National Laboratory with the U.S. Department of Energy/National Nuclear
 * Security Administration. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of TRIAD, Los Alamos National Laboratory, LANL, the
 *    U.S. Government, nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "c2_env.h"
#include "c2_io.h"

#include <fastbit/ibin.h>
#include <iostream>
#include <vector>

namespace c2 {

struct IndexBuilderOptions {
  IndexBuilderOptions() : ibis_col(NULL) {}
  ibis::column* ibis_col;
};

class IndexBuilder : public ibis::bin {
 public:
  IndexBuilder(const IndexBuilderOptions& options, WritableFile* file);
  ~IndexBuilder();

  template <typename T>
  void TEST_GranuleBuild(const std::vector<T>& data);
  template <typename T>
  void TEST_Build(const std::vector<T>& data);
  uint32_t DiskStorageUsage() const;
  uint32_t NumBitVectors() const { return nobs; }
  uint32_t NumKeys(size_t i) const { return bits[i] ? bits[i]->cnt() : 0; }
  uint32_t MemoryUsage(size_t i) const {
    return bits[i] ? bits[i]->bytes() : 0;
  }
  uint32_t MemoryUsage() const;
  void Reset() { clear(); }
  Status Finish();

 private:
  IndexBuilderOptions options_;
  WritableFile* file_;

  // No copying allowed
  void operator=(const IndexBuilder& builder);
  IndexBuilder(const IndexBuilder&);
};

template <typename T>
void IndexBuilder::TEST_GranuleBuild(const std::vector<T>& in) {
  ibis::array_t<T> arr(const_cast<T*>(in.data()), in.size());
  granuleMap gmap;
  uint64_t a1 = env::CurrentMicros();
  mapGranules(arr, gmap);
  uint64_t a2 = env::CurrentMicros();
  fprintf(stderr, "== Map granule: %.3f s\n", double(a2 - a1) / 1000 / 1000);
  // printGranules(std::cerr, gmap);
  convertGranules(gmap);
  nrows = arr.size();
}

template <typename T>
void IndexBuilder::TEST_Build(const std::vector<T>& in) {
  ibis::array_t<T> arr(const_cast<T*>(in.data()), in.size());
  uint64_t a1 = env::CurrentMicros();
  setBoundaries(arr);
  uint64_t a2 = env::CurrentMicros();
  fprintf(stderr, "== Make bounds: %.3f s\n", double(a2 - a1) / 1000 / 1000);
  binning(arr);
}

}  // namespace c2
