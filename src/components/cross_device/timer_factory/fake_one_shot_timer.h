// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_FAKE_ONE_SHOT_TIMER_H_
#define COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_FAKE_ONE_SHOT_TIMER_H_

#include "base/functional/callback_forward.h"
#include "base/timer/mock_timer.h"
#include "base/unguessable_token.h"

namespace cross_device {

// Fake base::OneShotTimer implementation, which extends MockTimer and provides
// a mechanism for alerting its creator when it is destroyed.
class FakeOneShotTimer : public base::MockOneShotTimer {
 public:
  FakeOneShotTimer(base::OnceCallback<void(const base::UnguessableToken&)>
                       destructor_callback);

  FakeOneShotTimer(const FakeOneShotTimer&) = delete;
  FakeOneShotTimer& operator=(const FakeOneShotTimer&) = delete;

  ~FakeOneShotTimer() override;

  const base::UnguessableToken& id() const { return id_; }

 private:
  base::OnceCallback<void(const base::UnguessableToken&)> destructor_callback_;
  base::UnguessableToken id_;
};

}  // namespace cross_device

#endif  // COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_FAKE_ONE_SHOT_TIMER_H_
