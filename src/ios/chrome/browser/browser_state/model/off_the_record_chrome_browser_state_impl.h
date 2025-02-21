// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_BROWSER_STATE_MODEL_OFF_THE_RECORD_CHROME_BROWSER_STATE_IMPL_H_
#define IOS_CHROME_BROWSER_BROWSER_STATE_MODEL_OFF_THE_RECORD_CHROME_BROWSER_STATE_IMPL_H_

#import "base/memory/raw_ptr.h"
#include "base/task/sequenced_task_runner.h"
#include "base/time/time.h"
#include "ios/chrome/browser/browser_state/model/off_the_record_chrome_browser_state_io_data.h"
#include "ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"

namespace sync_preferences {
class PrefServiceSyncable;
}

namespace policy {
class UserCloudPolicyManager;
}

// The implementation of ChromeBrowserState that is used for incognito browsing.
// Each OffTheRecordChromeBrowserStateImpl instance is associated with and owned
// by a non-incognito ChromeBrowserState instance.
class OffTheRecordChromeBrowserStateImpl final : public ChromeBrowserState {
 public:
  OffTheRecordChromeBrowserStateImpl(
      const OffTheRecordChromeBrowserStateImpl&) = delete;
  OffTheRecordChromeBrowserStateImpl& operator=(
      const OffTheRecordChromeBrowserStateImpl&) = delete;

  ~OffTheRecordChromeBrowserStateImpl() override;

  // ChromeBrowserState:
  ChromeBrowserState* GetOriginalChromeBrowserState() override;
  bool HasOffTheRecordChromeBrowserState() const override;
  ChromeBrowserState* GetOffTheRecordChromeBrowserState() override;
  void DestroyOffTheRecordChromeBrowserState() override;
  PrefProxyConfigTracker* GetProxyConfigTracker() override;
  BrowserStatePolicyConnector* GetPolicyConnector() override;
  policy::UserCloudPolicyManager* GetUserCloudPolicyManager() override;
  sync_preferences::PrefServiceSyncable* GetSyncablePrefs() override;
  ChromeBrowserStateIOData* GetIOData() override;
  void ClearNetworkingHistorySince(base::Time time,
                                   base::OnceClosure completion) override;
  net::URLRequestContextGetter* CreateRequestContext(
      ProtocolHandlerMap* protocol_handlers) override;
  base::WeakPtr<ChromeBrowserState> AsWeakPtr() override;

  // BrowserState:
  bool IsOffTheRecord() const override;

 private:
  friend class ChromeBrowserStateImpl;

  // `original_chrome_browser_state_` is the non-incognito
  // ChromeBrowserState instance that owns this instance.
  OffTheRecordChromeBrowserStateImpl(
      scoped_refptr<base::SequencedTaskRunner> io_task_runner,
      ChromeBrowserState* original_chrome_browser_state,
      const base::FilePath& otr_path);

  raw_ptr<ChromeBrowserState> original_chrome_browser_state_;  // weak

  // Creation time of the off-the-record BrowserState.
  const base::Time start_time_;

  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs_;

  std::unique_ptr<OffTheRecordChromeBrowserStateIOData::Handle> io_data_;
  std::unique_ptr<PrefProxyConfigTracker> pref_proxy_config_tracker_;

  base::WeakPtrFactory<OffTheRecordChromeBrowserStateImpl> weak_ptr_factory_{
      this};
};

#endif  // IOS_CHROME_BROWSER_BROWSER_STATE_MODEL_OFF_THE_RECORD_CHROME_BROWSER_STATE_IMPL_H_
