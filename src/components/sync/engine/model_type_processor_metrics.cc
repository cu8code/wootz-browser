// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine/model_type_processor_metrics.h"

#include <string>

#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_functions.h"
#include "base/strings/stringprintf.h"

namespace syncer {

void LogModelTypeConfigurationTime(ModelType model_type,
                                   SyncMode mode,
                                   base::Time configuration_start_time) {
  const base::TimeDelta configuration_duration =
      base::Time::Now() - configuration_start_time;

  base::UmaHistogramCustomTimes(
      base::StringPrintf(
          "Sync.ModelTypeConfigurationTime.%s.%s",
          (mode == SyncMode::kTransportOnly) ? "Ephemeral" : "Persistent",
          ModelTypeToHistogramSuffix(model_type)),
      configuration_duration,
      /*min=*/base::Milliseconds(1),
      /*max=*/base::Seconds(60),
      /*buckets=*/50);
}

void LogNonReflectionUpdateFreshnessToUma(ModelType type,
                                          base::Time remote_modification_time) {
  const base::TimeDelta freshness =
      base::Time::Now() - remote_modification_time;

  base::UmaHistogramCustomTimes(
      "Sync.NonReflectionUpdateFreshnessPossiblySkewed2", freshness,
      /*min=*/base::Milliseconds(100),
      /*max=*/base::Days(7),
      /*buckets=*/50);

  base::UmaHistogramCustomTimes(
      std::string("Sync.NonReflectionUpdateFreshnessPossiblySkewed2.") +
          ModelTypeToHistogramSuffix(type),
      freshness,
      /*min=*/base::Milliseconds(100),
      /*max=*/base::Days(7),
      /*buckets=*/50);
}

void LogClearMetadataWhileStoppedHistogram(ModelType model_type,
                                           bool is_delayed_call) {
  base::UmaHistogramEnumeration("Sync.ClearMetadataWhileStopped",
                                ModelTypeHistogramValue(model_type));
  const char* histogram_name =
      is_delayed_call ? "Sync.ClearMetadataWhileStopped.DelayedClear"
                      : "Sync.ClearMetadataWhileStopped.ImmediateClear";
  base::UmaHistogramEnumeration(histogram_name,
                                ModelTypeHistogramValue(model_type));
}

}  // namespace syncer
