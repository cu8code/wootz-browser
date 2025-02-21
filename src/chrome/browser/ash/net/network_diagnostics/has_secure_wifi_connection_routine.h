// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_NET_NETWORK_DIAGNOSTICS_HAS_SECURE_WIFI_CONNECTION_ROUTINE_H_
#define CHROME_BROWSER_ASH_NET_NETWORK_DIAGNOSTICS_HAS_SECURE_WIFI_CONNECTION_ROUTINE_H_

#include <vector>

#include "base/functional/callback.h"
#include "chrome/browser/ash/net/network_diagnostics/network_diagnostics_routine.h"
#include "chromeos/services/network_config/public/mojom/cros_network_config.mojom.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace ash {
namespace network_diagnostics {

// Tests whether the WiFi connection uses a secure encryption method.
class HasSecureWiFiConnectionRoutine : public NetworkDiagnosticsRoutine {
 public:
  explicit HasSecureWiFiConnectionRoutine(
      chromeos::network_diagnostics::mojom::RoutineCallSource source);
  HasSecureWiFiConnectionRoutine(const HasSecureWiFiConnectionRoutine&) =
      delete;
  HasSecureWiFiConnectionRoutine& operator=(
      const HasSecureWiFiConnectionRoutine&) = delete;
  ~HasSecureWiFiConnectionRoutine() override;

  // NetworkDiagnosticsRoutine:
  bool CanRun() override;
  chromeos::network_diagnostics::mojom::RoutineType Type() override;
  void Run() override;
  void AnalyzeResultsAndExecuteCallback() override;

 private:
  void FetchActiveWiFiNetworks();
  void OnNetworkStateListReceived(
      std::vector<chromeos::network_config::mojom::NetworkStatePropertiesPtr>
          networks);

  mojo::Remote<chromeos::network_config::mojom::CrosNetworkConfig>
      remote_cros_network_config_;
  bool wifi_connected_ = false;
  chromeos::network_config::mojom::SecurityType wifi_security_ =
      chromeos::network_config::mojom::SecurityType::kNone;
  std::vector<
      chromeos::network_diagnostics::mojom::HasSecureWiFiConnectionProblem>
      problems_;
};

}  // namespace network_diagnostics
}  // namespace ash

#endif  // CHROME_BROWSER_ASH_NET_NETWORK_DIAGNOSTICS_HAS_SECURE_WIFI_CONNECTION_ROUTINE_H_
