// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/side_panel/read_anything/read_anything_controller.h"

#include <memory>

#include "base/test/gtest_util.h"
#include "chrome/browser/ui/views/frame/test_with_browser_view.h"
#include "chrome/browser/ui/views/side_panel/read_anything/read_anything_model.h"
#include "chrome/browser/ui/webui/side_panel/read_anything/read_anything_prefs.h"
#include "chrome/common/accessibility/read_anything_constants.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/ui_test_utils.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "ui/accessibility/accessibility_features.h"

using testing::_;
class MockReadAnythingModelObserver : public ReadAnythingModel::Observer {
 public:
  MOCK_METHOD(void,
              OnReadAnythingThemeChanged,
              (const std::string& font_name,
               double font_scale,
               bool links_enabled,
               ui::ColorId foreground_color_id,
               ui::ColorId background_color_id,
               ui::ColorId separator_color_id,
               ui::ColorId dropdown_color_id,
               ui::ColorId selection_color_id,
               ui::ColorId focus_ring_color_id,
               read_anything::mojom::LineSpacing line_spacing,
               read_anything::mojom::LetterSpacing letter_spacing),
              (override));
};

class ReadAnythingControllerTest : public TestWithBrowserView {
 public:
  void SetUp() override {
    scoped_feature_list_.InitWithFeatures(
        {features::kReadAnythingLocalSidePanel,
         features::kReadAnythingWebUIToolbar},
        {});
    TestWithBrowserView::SetUp();

    model_ = std::make_unique<ReadAnythingModel>();
    controller_ =
        std::make_unique<ReadAnythingController>(model_.get(), browser());
    auto* web_contents_ = browser()->OpenURL(
        content::OpenURLParams(GURL("https://google.com"), content::Referrer(),
                               WindowOpenDisposition::CURRENT_TAB,
                               ui::PAGE_TRANSITION_TYPED, false),
        /*navigation_handle_callback=*/{});
    controller_for_web_contents_ =
        std::make_unique<ReadAnythingController>(model_.get(), web_contents_);

    // Reset prefs to default values for test.
    browser()->profile()->GetPrefs()->SetString(
        prefs::kAccessibilityReadAnythingFontName,
        string_constants::kReadAnythingPlaceholderFontName);
    browser()->profile()->GetPrefs()->SetDouble(
        prefs::kAccessibilityReadAnythingFontScale,
        kReadAnythingDefaultFontScale);
    browser()->profile()->GetPrefs()->SetInteger(
        prefs::kAccessibilityReadAnythingColorInfo,
        static_cast<int>(read_anything::mojom::Colors::kDefaultValue));
    browser()->profile()->GetPrefs()->SetInteger(
        prefs::kAccessibilityReadAnythingLineSpacing,
        static_cast<int>(read_anything::mojom::LineSpacing::kDefaultValue));
    browser()->profile()->GetPrefs()->SetInteger(
        prefs::kAccessibilityReadAnythingLetterSpacing,
        static_cast<int>(read_anything::mojom::LetterSpacing::kDefaultValue));
    browser()->profile()->GetPrefs()->SetBoolean(
        prefs::kAccessibilityReadAnythingLinksEnabled,
        kReadAnythingDefaultLinksEnabled);
  }

  void TearDown() override {
    controller_for_web_contents_ = nullptr;
    controller_ = nullptr;
    TestWithBrowserView::TearDown();
  }

  void MockOnFontChoiceChanged(int index) {
    controller_->OnFontChoiceChanged(index);
  }

  void MockControllerForWebContentsOnFontChoiceChanged(int index) {
    controller_for_web_contents_->OnFontChoiceChanged(index);
  }

  void MockOnFontSizeChanged(bool increase) {
    controller_->OnFontSizeChanged(increase);
  }

  void MockControllerForWebContentsOnFontSizeChanged(bool increase) {
    controller_for_web_contents_->OnFontSizeChanged(increase);
  }

  void MockOnColorsChanged(int index) { controller_->OnColorsChanged(index); }

  void MockControllerForWebContentsOnColorsChanged(int index) {
    controller_for_web_contents_->OnColorsChanged(index);
  }

  void MockOnLineSpacingChanged(int index) {
    controller_->OnLineSpacingChanged(index);
  }

  void MockControllerForWebContentsOnLineSpacingChanged(int index) {
    controller_for_web_contents_->OnLineSpacingChanged(index);
  }

  void MockOnLetterSpacingChanged(int index) {
    controller_->OnLetterSpacingChanged(index);
  }

  void MockOnLinksEnabledChanged(bool enabled) {
    controller_->OnLinksEnabledChanged(enabled);
  }

  void MockControllerForWebContentsOnLetterSpacingChanged(int index) {
    controller_for_web_contents_->OnLetterSpacingChanged(index);
  }

  void MockModelInit(std::string language,
                     std::string font_name,
                     double font_scale,
                     bool links_enabled,
                     read_anything::mojom::Colors colors,
                     read_anything::mojom::LineSpacing line_spacing,
                     read_anything::mojom::LetterSpacing letter_spacing) {
    model_->Init(language, font_name, font_scale, links_enabled, colors,
                 line_spacing, letter_spacing);
  }

  std::string GetPrefFontName() {
    return browser()->profile()->GetPrefs()->GetString(
        prefs::kAccessibilityReadAnythingFontName);
  }

  double GetPrefFontScale() {
    return browser()->profile()->GetPrefs()->GetDouble(
        prefs::kAccessibilityReadAnythingFontScale);
  }

  int GetPrefsColors() {
    return browser()->profile()->GetPrefs()->GetInteger(
        prefs::kAccessibilityReadAnythingColorInfo);
  }

  int GetPrefsLineSpacing() {
    return browser()->profile()->GetPrefs()->GetInteger(
        prefs::kAccessibilityReadAnythingLineSpacing);
  }

  int GetPrefsLetterSpacing() {
    return browser()->profile()->GetPrefs()->GetInteger(
        prefs::kAccessibilityReadAnythingLetterSpacing);
  }

  bool GetPrefsLinksEnabled() {
    return browser()->profile()->GetPrefs()->GetBoolean(
        prefs::kAccessibilityReadAnythingLinksEnabled);
  }

  TabStripModel* GetTabStripModel() { return browser()->tab_strip_model(); }

 protected:
  std::unique_ptr<ReadAnythingModel> model_;
  std::unique_ptr<ReadAnythingController> controller_;
  MockReadAnythingModelObserver model_observer_;

  // Variables for the controller created using web contents;
  std::unique_ptr<ReadAnythingController> controller_for_web_contents_;
  base::test::ScopedFeatureList scoped_feature_list_;
};

TEST_F(ReadAnythingControllerTest, ValidIndexUpdatesFontNamePref) {
  std::string expected_font_name = "Comic Neue";

  // Initialize model with English so all fonts are available choices.
  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 4.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);
  MockOnFontChoiceChanged(3);

  EXPECT_EQ(expected_font_name, GetPrefFontName());
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsValidIndexUpdatesFontNamePref) {
  std::string expected_font_name = "Comic Neue";

  // Initialize model with English so all fonts are available choices.
  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 4.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);
  MockControllerForWebContentsOnFontChoiceChanged(3);

  EXPECT_EQ(expected_font_name, GetPrefFontName());
}

TEST_F(ReadAnythingControllerTest, OnFontSizeChangedIncreaseUpdatesPref) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  MockOnFontSizeChanged(true);

  EXPECT_NEAR(GetPrefFontScale(), 1.25, 0.01);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnFontSizeChangedIncreaseUpdatesPref) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  MockControllerForWebContentsOnFontSizeChanged(true);

  EXPECT_NEAR(GetPrefFontScale(), 1.25, 0.01);
}

TEST_F(ReadAnythingControllerTest, OnFontSizeChangedDecreasePref) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  MockOnFontSizeChanged(false);

  EXPECT_NEAR(GetPrefFontScale(), 0.75, 0.01);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnFontSizeChangedDecreasePref) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  MockControllerForWebContentsOnFontSizeChanged(false);

  EXPECT_NEAR(GetPrefFontScale(), 0.75, 0.01);
}

TEST_F(ReadAnythingControllerTest, OnFontSizeChangedHonorsMax) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 4.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);

  MockOnFontSizeChanged(true);

  EXPECT_NEAR(GetPrefFontScale(), 4.5, 0.01);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnFontSizeChangedHonorsMax) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 4.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);

  MockControllerForWebContentsOnFontSizeChanged(true);

  EXPECT_NEAR(GetPrefFontScale(), 4.5, 0.01);
}

TEST_F(ReadAnythingControllerTest, OnFontSizeChangedHonorsMin) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 0.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);

  MockOnFontSizeChanged(false);

  EXPECT_NEAR(GetPrefFontScale(), 0.5, 0.01);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnFontSizeChangedHonorsMin) {
  EXPECT_NEAR(GetPrefFontScale(), 1.0, 0.01);

  std::string font_name;
  std::string language = "en";
  MockModelInit(language, font_name, 0.5, true,
                read_anything::mojom::Colors::kDefaultValue,
                read_anything::mojom::LineSpacing::kDefaultValue,
                read_anything::mojom::LetterSpacing::kDefaultValue);

  MockControllerForWebContentsOnFontSizeChanged(false);

  EXPECT_NEAR(GetPrefFontScale(), 0.5, 0.01);
}

TEST_F(ReadAnythingControllerTest, OnColorsChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsColors(), 0);

  MockOnColorsChanged(static_cast<int>(read_anything::mojom::Colors::kYellow));

  EXPECT_EQ(GetPrefsColors(), 3);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnColorsChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsColors(), 0);

  MockControllerForWebContentsOnColorsChanged(
      static_cast<int>(read_anything::mojom::Colors::kYellow));

  EXPECT_EQ(GetPrefsColors(), 3);
}

TEST_F(ReadAnythingControllerTest, OnLineSpacingChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kStandard) - 1);

  EXPECT_EQ(GetPrefsLineSpacing(), 1);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLineSpacingChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockControllerForWebContentsOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kStandard) - 1);

  EXPECT_EQ(GetPrefsLineSpacing(), 1);
}

TEST_F(ReadAnythingControllerTest,
       OnLineSpacingChangedValidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kVeryLoose) - 1);

  EXPECT_EQ(GetPrefsLineSpacing(), 3);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLineSpacingChangedValidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockControllerForWebContentsOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kVeryLoose) - 1);

  EXPECT_EQ(GetPrefsLineSpacing(), 3);
}

TEST_F(ReadAnythingControllerTest,
       OnLineSpacingChangedInvalidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kVeryLoose));

  EXPECT_EQ(GetPrefsLineSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLineSpacingChangedInvalidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockControllerForWebContentsOnLineSpacingChanged(
      static_cast<int>(read_anything::mojom::LineSpacing::kVeryLoose));

  EXPECT_EQ(GetPrefsLineSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest, OnLineSpacingChangedInvalidInput) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  MockOnLineSpacingChanged(10);

  EXPECT_EQ(GetPrefsLineSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLineSpacingChangedInvalidInput) {
  EXPECT_EQ(GetPrefsLineSpacing(), 2);

  MockControllerForWebContentsOnLineSpacingChanged(10);

  EXPECT_EQ(GetPrefsLineSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest, OnLetterSpacingChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kWide) - 1);

  EXPECT_EQ(GetPrefsLetterSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLetterSpacingChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockControllerForWebContentsOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kWide) - 1);

  EXPECT_EQ(GetPrefsLetterSpacing(), 2);
}

TEST_F(ReadAnythingControllerTest,
       OnLetterSpacingChangedValidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kVeryWide) - 1);

  EXPECT_EQ(GetPrefsLetterSpacing(), 3);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLetterSpacingChangedValidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Subtract one to account for the deprecated value (kLooseDeprecated), since
  // this is the index in the drop-down and not the enum value.
  MockControllerForWebContentsOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kVeryWide) - 1);

  EXPECT_EQ(GetPrefsLetterSpacing(), 3);
}

TEST_F(ReadAnythingControllerTest,
       OnLetterSpacingChangedInvalidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Since this is the index in the drop-down and not the enum value, the max
  // enum value is one larger than the max index value in the drop down.
  MockOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kVeryWide));

  EXPECT_EQ(GetPrefsLetterSpacing(), 1);
}

TEST_F(
    ReadAnythingControllerTest,
    ControllerForWebContentsOnLetterSpacingChangedInvalidInputAtTopBoundary) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  // Since this is the index in the drop-down and not the enum value, the max
  // enum value is one larger than the max index value in the drop down.
  MockControllerForWebContentsOnLetterSpacingChanged(
      static_cast<int>(read_anything::mojom::LetterSpacing::kVeryWide));

  EXPECT_EQ(GetPrefsLetterSpacing(), 1);
}

TEST_F(ReadAnythingControllerTest, OnLetterSpacingChangedInvalidInput) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  MockOnLetterSpacingChanged(10);

  EXPECT_EQ(GetPrefsLetterSpacing(), 1);
}

TEST_F(ReadAnythingControllerTest,
       ControllerForWebContentsOnLetterSpacingChangedInvalidInput) {
  EXPECT_EQ(GetPrefsLetterSpacing(), 1);

  MockControllerForWebContentsOnLetterSpacingChanged(10);

  EXPECT_EQ(GetPrefsLetterSpacing(), 1);
}

TEST_F(ReadAnythingControllerTest, OnLinksEnabledChangedUpdatesPref) {
  EXPECT_EQ(GetPrefsLinksEnabled(), true);

  MockOnLinksEnabledChanged(false);

  EXPECT_EQ(GetPrefsLinksEnabled(), false);
}
