// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/mahi/media_app/mahi_media_app_events_proxy_impl.h"

namespace ash {

MahiMediaAppEventsProxyImpl::MahiMediaAppEventsProxyImpl() = default;
MahiMediaAppEventsProxyImpl::~MahiMediaAppEventsProxyImpl() = default;

void MahiMediaAppEventsProxyImpl::OnPdfGetFocus(
    const base::UnguessableToken client_id) {
  for (auto& observer : observers_) {
    observer.OnPdfGetFocus(client_id);
  }
}

void MahiMediaAppEventsProxyImpl::OnPdfContextMenuShown(
    base::UnguessableToken client_id,
    const gfx::Rect& anchor_bounds) {
  for (auto& observer : observers_) {
    observer.OnPdfContextMenuShown(anchor_bounds);
  }
}

void MahiMediaAppEventsProxyImpl::OnPdfContextMenuHide() {
  for (auto& observer : observers_) {
    observer.OnPdfContextMenuHide();
  }
}

void MahiMediaAppEventsProxyImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void MahiMediaAppEventsProxyImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

}  // namespace ash
