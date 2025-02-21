// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_INTERNAL_AUTOFILL_WEB_VIEW_AUTOFILL_CLIENT_IOS_H_
#define IOS_WEB_VIEW_INTERNAL_AUTOFILL_WEB_VIEW_AUTOFILL_CLIENT_IOS_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "components/autofill/core/browser/autocomplete_history_manager.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "components/autofill/core/browser/crowdsourcing/autofill_crowdsourcing_manager.h"
#include "components/autofill/core/browser/logging/log_manager.h"
#include "components/autofill/core/browser/payments/legal_message_line.h"
#include "components/autofill/core/browser/personal_data_manager.h"
#include "components/autofill/core/browser/strike_databases/strike_database.h"
#include "components/prefs/pref_service.h"
#include "components/sync/service/sync_service.h"
#import "ios/web/public/web_state.h"
#import "ios/web_view/internal/autofill/cwv_autofill_client_ios_bridge.h"
#include "ios/web_view/internal/autofill/ios_web_view_payments_autofill_client.h"
#include "ios/web_view/internal/web_view_browser_state.h"

namespace autofill {

namespace payments {
class IOSWebViewPaymentsAutofillClient;
}  // namespace payments

// WebView implementation of AutofillClient.
class WebViewAutofillClientIOS : public AutofillClient {
 public:
  static std::unique_ptr<WebViewAutofillClientIOS> Create(
      web::WebState* web_state,
      ios_web_view::WebViewBrowserState* browser_state);

  WebViewAutofillClientIOS(
      PrefService* pref_service,
      PersonalDataManager* personal_data_manager,
      AutocompleteHistoryManager* autocomplete_history_manager,
      web::WebState* web_state,
      signin::IdentityManager* identity_manager,
      StrikeDatabase* strike_database,
      syncer::SyncService* sync_service,
      std::unique_ptr<autofill::LogManager> log_manager);

  WebViewAutofillClientIOS(const WebViewAutofillClientIOS&) = delete;
  WebViewAutofillClientIOS& operator=(const WebViewAutofillClientIOS&) = delete;

  ~WebViewAutofillClientIOS() override;

  // AutofillClient:
  bool IsOffTheRecord() const override;
  scoped_refptr<network::SharedURLLoaderFactory> GetURLLoaderFactory() override;
  AutofillCrowdsourcingManager* GetCrowdsourcingManager() override;
  PersonalDataManager* GetPersonalDataManager() override;
  AutocompleteHistoryManager* GetAutocompleteHistoryManager() override;
  PrefService* GetPrefs() override;
  const PrefService* GetPrefs() const override;
  syncer::SyncService* GetSyncService() override;
  signin::IdentityManager* GetIdentityManager() override;
  FormDataImporter* GetFormDataImporter() override;
  payments::PaymentsAutofillClient* GetPaymentsAutofillClient() override;
  StrikeDatabase* GetStrikeDatabase() override;
  ukm::UkmRecorder* GetUkmRecorder() override;
  ukm::SourceId GetUkmSourceId() override;
  AddressNormalizer* GetAddressNormalizer() override;
  const GURL& GetLastCommittedPrimaryMainFrameURL() const override;
  url::Origin GetLastCommittedPrimaryMainFrameOrigin() const override;
  security_state::SecurityLevel GetSecurityLevelForUmaHistograms() override;
  const translate::LanguageState* GetLanguageState() override;
  translate::TranslateDriver* GetTranslateDriver() override;
  void ShowAutofillSettings(FillingProduct main_filling_product) override;
  void ConfirmSaveCreditCardToCloud(
      const CreditCard& card,
      const LegalMessageLines& legal_message_lines,
      SaveCreditCardOptions options,
      UploadSaveCardPromptCallback callback) override;
  void ConfirmCreditCardFillAssist(const CreditCard& card,
                                   base::OnceClosure callback) override;
  void ConfirmSaveAddressProfile(
      const AutofillProfile& profile,
      const AutofillProfile* original_profile,
      SaveAddressProfilePromptOptions options,
      AddressProfileSavePromptCallback callback) override;
  void ShowEditAddressProfileDialog(
      const AutofillProfile& profile,
      AddressProfileSavePromptCallback on_user_decision_callback) override;
  void ShowDeleteAddressProfileDialog(
      const AutofillProfile& profile,
      AddressProfileDeleteDialogCallback delete_dialog_callback) override;
  bool HasCreditCardScanFeature() const override;
  void ScanCreditCard(CreditCardScanCallback callback) override;
  bool ShowTouchToFillCreditCard(
      base::WeakPtr<TouchToFillDelegate> delegate,
      base::span<const autofill::CreditCard> cards_to_suggest) override;
  void HideTouchToFillCreditCard() override;
  void ShowAutofillSuggestions(
      const AutofillClient::PopupOpenArgs& open_args,
      base::WeakPtr<AutofillSuggestionDelegate> delegate) override;
  void UpdateAutofillDataListValues(
      base::span<const autofill::SelectOption> datalist) override;
  void PinAutofillSuggestions() override;
  void UpdatePopup(const std::vector<Suggestion>& suggestions,
                   FillingProduct main_filling_product,
                   AutofillSuggestionTriggerSource trigger_source) override;
  void HideAutofillSuggestions(SuggestionHidingReason reason) override;
  bool IsAutocompleteEnabled() const override;
  bool IsPasswordManagerEnabled() override;
  void DidFillOrPreviewForm(mojom::ActionPersistence action_persistence,
                            AutofillTriggerSource trigger_source,
                            bool is_refill) override;
  void DidFillOrPreviewField(const std::u16string& autofilled_value,
                             const std::u16string& profile_full_name) override;
  bool IsContextSecure() const override;
  void OpenPromoCodeOfferDetailsURL(const GURL& url) override;
  autofill::FormInteractionsFlowId GetCurrentFormInteractionsFlowId() override;
  bool IsLastQueriedField(FieldGlobalId field_id) override;

  LogManager* GetLogManager() const override;

  void set_bridge(id<CWVAutofillClientIOSBridge> bridge);

 private:
  PrefService* pref_service_;
  std::unique_ptr<AutofillCrowdsourcingManager> crowdsourcing_manager_;
  PersonalDataManager* personal_data_manager_;
  AutocompleteHistoryManager* autocomplete_history_manager_;
  web::WebState* web_state_;
  __weak id<CWVAutofillClientIOSBridge> bridge_;
  signin::IdentityManager* identity_manager_;
  std::unique_ptr<payments::IOSWebViewPaymentsAutofillClient>
      payments_autofill_client_;
  std::unique_ptr<FormDataImporter> form_data_importer_;
  StrikeDatabase* strike_database_;
  syncer::SyncService* sync_service_ = nullptr;
  std::unique_ptr<LogManager> log_manager_;
};

}  // namespace autofill

#endif  // IOS_WEB_VIEW_INTERNAL_AUTOFILL_WEB_VIEW_AUTOFILL_CLIENT_IOS_H_
