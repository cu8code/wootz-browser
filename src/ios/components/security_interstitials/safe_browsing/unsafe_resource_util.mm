// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/components/security_interstitials/safe_browsing/unsafe_resource_util.h"

#import "components/safe_browsing/ios/browser/safe_browsing_url_allow_list.h"
#import "ios/web/public/web_state.h"
#import "services/network/public/mojom/fetch_api.mojom.h"

using safe_browsing::ThreatPatternType;
using security_interstitials::BaseSafeBrowsingErrorUI;
using security_interstitials::UnsafeResource;

void RunUnsafeResourceCallback(const UnsafeResource& resource,
                               bool proceed,
                               bool showed_interstitial) {
  DCHECK(resource.callback_sequence);
  DCHECK(!resource.callback.is_null());
  UnsafeResource::UrlCheckResult result(
      proceed, showed_interstitial,
      /*has_post_commit_interstitial_skipped=*/false);
  resource.callback_sequence->PostTask(
      FROM_HERE, base::BindOnce(resource.callback, result));
}

BaseSafeBrowsingErrorUI::SBInterstitialReason
GetUnsafeResourceInterstitialReason(const UnsafeResource& resource) {
  switch (resource.threat_type) {
    case safe_browsing::SBThreatType::SB_THREAT_TYPE_BILLING:
      return BaseSafeBrowsingErrorUI::SB_REASON_BILLING;
    case safe_browsing::SBThreatType::SB_THREAT_TYPE_URL_MALWARE:
      return BaseSafeBrowsingErrorUI::SB_REASON_MALWARE;
    case safe_browsing::SBThreatType::SB_THREAT_TYPE_URL_UNWANTED:
      return BaseSafeBrowsingErrorUI::SB_REASON_HARMFUL;
    default:
      return BaseSafeBrowsingErrorUI::SB_REASON_PHISHING;
  }
}

std::string GetUnsafeResourceMetricPrefix(
    const security_interstitials::UnsafeResource& resource) {
  std::string prefix;
  switch (GetUnsafeResourceInterstitialReason(resource)) {
    case BaseSafeBrowsingErrorUI::SB_REASON_MALWARE:
      prefix = "malware";
      break;
    case BaseSafeBrowsingErrorUI::SB_REASON_HARMFUL:
      prefix = "harmful";
      break;
    case BaseSafeBrowsingErrorUI::SB_REASON_BILLING:
      prefix = "billing";
      break;
    case BaseSafeBrowsingErrorUI::SB_REASON_PHISHING:
      prefix = "phishing";
      break;
  }
  DCHECK(prefix.length());
  if (resource.is_subresource)
    prefix += "_subresource";
  return prefix;
}

SafeBrowsingUrlAllowList* GetAllowListForResource(
    const security_interstitials::UnsafeResource& resource) {
  web::WebState* web_state = resource.weak_web_state.get();
  if (!web_state)
    return nullptr;
  return SafeBrowsingUrlAllowList::FromWebState(web_state);
}

const GURL GetMainFrameUrl(
    const security_interstitials::UnsafeResource& resource) {
  if (resource.request_destination ==
      network::mojom::RequestDestination::kDocument)
    return resource.url;
  return resource.weak_web_state.get()->GetLastCommittedURL();
}
