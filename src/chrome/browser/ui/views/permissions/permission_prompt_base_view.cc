// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/permissions/permission_prompt_base_view.h"

#include "chrome/browser/picture_in_picture/picture_in_picture_occlusion_tracker.h"
#include "chrome/browser/picture_in_picture/picture_in_picture_window_manager.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/views/title_origin_label.h"
#include "chrome/grit/generated_resources.h"
#include "components/permissions/features.h"
#include "components/strings/grit/components_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/window/dialog_client_view.h"

namespace {

constexpr UrlIdentity::TypeSet allowed_types = {
    UrlIdentity::Type::kDefault, UrlIdentity::Type::kChromeExtension,
    UrlIdentity::Type::kIsolatedWebApp, UrlIdentity::Type::kFile};

constexpr UrlIdentity::FormatOptions options = {
    .default_options = {
        UrlIdentity::DefaultFormatOptions::kOmitCryptographicScheme}};

}  // namespace

PermissionPromptBaseView::PermissionPromptBaseView(
    Browser* browser,
    base::WeakPtr<permissions::PermissionPrompt::Delegate> delegate)
    : BubbleDialogDelegateView(/*anchor_view=*/nullptr,
                               views::BubbleBorder::TOP_LEFT,
                               views::BubbleBorder::DIALOG_SHADOW,
                               /*autosize=*/true),
      url_identity_(GetUrlIdentity(browser, *delegate)),
      is_for_picture_in_picture_window_(browser &&
                                        browser->is_type_picture_in_picture()) {
  // To prevent permissions being accepted accidentally, and as a security
  // measure against crbug.com/619429, permission prompts should not be accepted
  // as the default action.
  SetDefaultButton(ui::DIALOG_BUTTON_NONE);
}

void PermissionPromptBaseView::AddedToWidget() {
  if (url_identity_.type == UrlIdentity::Type::kDefault) {
    // There is a risk of URL spoofing from origins that are too wide to fit in
    // the bubble; elide origins from the front to prevent this.
    GetBubbleFrameView()->SetTitleView(
        CreateTitleOriginLabel(GetWindowTitle()));
  }

  // If we're for a picture-in-picture window, then we are in an always-on-top
  // widget that should be tracked by the PictureInPictureOcclusionTracker.
  if (is_for_picture_in_picture_window_) {
    PictureInPictureOcclusionTracker* tracker =
        PictureInPictureWindowManager::GetInstance()->GetOcclusionTracker();
    if (tracker) {
      tracker->OnPictureInPictureWidgetOpened(GetWidget());
    }
  }

  // Either way, we want to know if we're ever occluded by an always-on-top
  // window.
  occlusion_observation_.Observe(GetWidget());
}

bool PermissionPromptBaseView::ShouldIgnoreButtonPressedEventHandling(
    View* button,
    const ui::Event& event) const {
  // Ignore button pressed events whenever we're occluded by a
  // picture-in-picture window.
  return occluded_by_picture_in_picture_;
}

void PermissionPromptBaseView::OnOcclusionStateChanged(bool occluded) {
  occluded_by_picture_in_picture_ = occluded;
}

void PermissionPromptBaseView::FilterUnintenedEventsAndRunCallbacks(
    int button_id,
    const ui::Event& event) {
  if (GetDialogClientView()->IsPossiblyUnintendedInteraction(event)) {
    return;
  }

  View* button = AsDialogDelegate()->GetExtraView()->GetViewByID(button_id);

  if (ShouldIgnoreButtonPressedEventHandling(button, event)) {
    return;
  }

  RunButtonCallback(button_id);
}

// static
UrlIdentity PermissionPromptBaseView::GetUrlIdentity(
    Browser* browser,
    permissions::PermissionPrompt::Delegate& delegate) {
  DCHECK(!delegate.Requests().empty());
  GURL origin_url = delegate.GetRequestingOrigin();

  UrlIdentity url_identity =
      UrlIdentity::CreateFromUrl(browser ? browser->profile() : nullptr,
                                 origin_url, allowed_types, options);

  if (url_identity.type == UrlIdentity::Type::kFile) {
    // File URLs will show the same constant.
    url_identity.name =
        l10n_util::GetStringUTF16(IDS_PERMISSIONS_BUBBLE_PROMPT_THIS_FILE);
  }

  return url_identity;
}

std::u16string PermissionPromptBaseView::GetAllowAlwaysText(
    const std::vector<raw_ptr<permissions::PermissionRequest,
                              VectorExperimental>>& visible_requests) {
  CHECK_GT(visible_requests.size(), 0u);

  if (visible_requests.size() == 1 &&
      visible_requests[0]->GetAllowAlwaysText().has_value()) {
    // A prompt for a single request can use an "allow always" text that is
    // customized for it.
    return visible_requests[0]->GetAllowAlwaysText().value();
  }

  // Use the generic text.
  return l10n_util::GetStringUTF16(
      permissions::feature_params::kUseWhileVisitingLanguage.Get()
          ? IDS_PERMISSION_ALLOW_WHILE_VISITING
          : IDS_PERMISSION_ALLOW_EVERY_VISIT);
}

BEGIN_METADATA(PermissionPromptBaseView)
END_METADATA
