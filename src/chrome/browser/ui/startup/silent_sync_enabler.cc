// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/startup/silent_sync_enabler.h"

#include "base/functional/callback_forward.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/identity_manager_factory.h"
#include "chrome/browser/ui/webui/signin/login_ui_service.h"
#include "chrome/browser/ui/webui/signin/signin_utils.h"
#include "chrome/browser/ui/webui/signin/turn_sync_on_helper.h"
#include "components/signin/public/base/signin_metrics.h"
#include "components/signin/public/identity_manager/identity_manager.h"
#include "google_apis/gaia/core_account_id.h"

namespace {

// Silently goes through the `TurnSyncOnHelper` flow, enabling Sync when given
// the opportunity.
class SilentTurnSyncOnHelperDelegate : public TurnSyncOnHelper::Delegate {
 public:
  ~SilentTurnSyncOnHelperDelegate() override = default;

  // TurnSyncOnHelper::Delegate:
  void ShowEnterpriseAccountConfirmation(
      const AccountInfo& account_info,
      signin::SigninChoiceCallback callback) override {
    // We proceed here, and we are waiting until `ShowSyncConfirmation()` for
    // the Sync engine to be started to know if we can proceed or not.
    // TODO(crbug.com/40225086): Introduce a `DEFER` status or a new
    // `ShouldShowEnterpriseAccountConfirmation()` delegate method to handle
    // management consent not being handled at this step.
    std::move(callback).Run(signin::SIGNIN_CHOICE_CONTINUE);
  }

  void ShowSyncConfirmation(
      base::OnceCallback<void(LoginUIService::SyncConfirmationUIClosedResult)>
          callback) override {
    // The purpose of this delegate is specifically to enable Sync silently. If
    // we get all the way here, we assume that we can proceed with it.
    std::move(callback).Run(LoginUIService::SyncConfirmationUIClosedResult::
                                SYNC_WITH_DEFAULT_SETTINGS);
  }

  bool ShouldAbortBeforeShowSyncDisabledConfirmation() override { return true; }

  void ShowSyncDisabledConfirmation(
      bool is_managed_account,
      base::OnceCallback<void(LoginUIService::SyncConfirmationUIClosedResult)>
          callback) override {
    // If Sync is disabled, the `TurnSyncOnHelper` should quit earlier due to
    // `ShouldAbortBeforeShowSyncDisabledConfirmation()`.
    NOTREACHED_IN_MIGRATION();
  }

  void ShowLoginError(const SigninUIError& error) override {
    NOTREACHED_IN_MIGRATION();
  }

  void ShowMergeSyncDataConfirmation(const std::string&,
                                     const std::string&,
                                     signin::SigninChoiceCallback) override {
    NOTREACHED_IN_MIGRATION();
  }

  void ShowSyncSettings() override { NOTREACHED_IN_MIGRATION(); }

  void SwitchToProfile(Profile*) override { NOTREACHED_IN_MIGRATION(); }
};

}  // namespace

SilentSyncEnabler::SilentSyncEnabler(Profile& profile,
                                     signin::IdentityManager& identity_manager)
    : profile_(profile), identity_manager_(identity_manager) {}

SilentSyncEnabler::~SilentSyncEnabler() = default;

void SilentSyncEnabler::StartAttempt(base::OnceClosure callback) {
  DCHECK(!callback_);  // An attempt should not be ongoing
  DCHECK(callback);

  DCHECK(identity_manager_->HasPrimaryAccount(signin::ConsentLevel::kSignin));
  DCHECK(!identity_manager_->HasPrimaryAccount(signin::ConsentLevel::kSync));

  callback_ = std::move(callback);
  if (identity_manager_->AreRefreshTokensLoaded()) {
    TryEnableSyncSilentlyWithToken();
  } else {
    scoped_observation_.Observe(&identity_manager_.get());
  }
}

void SilentSyncEnabler::OnRefreshTokensLoaded() {
  scoped_observation_.Reset();
  TryEnableSyncSilentlyWithToken();
}

void SilentSyncEnabler::TryEnableSyncSilentlyWithToken() {
  CoreAccountId account_id =
      identity_manager_->GetPrimaryAccountId(signin::ConsentLevel::kSignin);
  if (!identity_manager_->HasAccountWithRefreshToken(account_id)) {
    // Still no token, just give up.
    if (callback_)
      std::move(callback_).Run();
    return;
  }

  // TurnSyncOnHelper deletes itself once done.
  new TurnSyncOnHelper(
      &profile_.get(), signin_metrics::AccessPoint::ACCESS_POINT_UNKNOWN,
      signin_metrics::PromoAction::PROMO_ACTION_NO_SIGNIN_PROMO, account_id,
      TurnSyncOnHelper::SigninAbortedMode::KEEP_ACCOUNT,
      std::make_unique<SilentTurnSyncOnHelperDelegate>(), std::move(callback_));
}
