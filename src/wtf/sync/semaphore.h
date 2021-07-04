// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_SYNCHRONIZATION_SEMAPHORE_H_
#define FLUTTER_FML_SYNCHRONIZATION_SEMAPHORE_H_

#include <memory>

#include "wtf/macros.h"

namespace wtf {

class PlatformSemaphore;

class Semaphore {
 public:
  explicit Semaphore(uint32_t count);

  ~Semaphore();

  bool IsValid() const;

  [[nodiscard]] bool TryWait();

  void Signal();

 private:
  std::unique_ptr<PlatformSemaphore> _impl;

  WTF_DISALLOW_COPY_AND_ASSIGN(Semaphore);
};

}  // namespace wtf

#endif  // FLUTTER_FML_SYNCHRONIZATION_SEMAPHORE_H_
