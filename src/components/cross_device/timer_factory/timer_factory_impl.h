// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_TIMER_FACTORY_IMPL_H_
#define COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_TIMER_FACTORY_IMPL_H_

#include <memory>

#include "components/cross_device/timer_factory/timer_factory.h"

namespace base {
class OneShotTimer;
}

namespace cross_device {

// Concrete TimerFactory implementation, which returns base::OneShotTimer
// objects.
class TimerFactoryImpl : public TimerFactory {
 public:
  class Factory {
   public:
    static std::unique_ptr<TimerFactory> Create();
    static void SetFactoryForTesting(std::unique_ptr<Factory> test_factory);
    virtual ~Factory();

   protected:
    virtual std::unique_ptr<TimerFactory> CreateInstance() = 0;

   private:
    static std::unique_ptr<Factory> test_factory_;
  };

  TimerFactoryImpl(const TimerFactoryImpl&) = delete;
  TimerFactoryImpl& operator=(const TimerFactoryImpl&) = delete;

  ~TimerFactoryImpl() override;

 private:
  TimerFactoryImpl();

  // TimerFactory:
  std::unique_ptr<base::OneShotTimer> CreateOneShotTimer() override;
};

}  // namespace cross_device

#endif  // COMPONENTS_CROSS_DEVICE_TIMER_FACTORY_TIMER_FACTORY_IMPL_H_
