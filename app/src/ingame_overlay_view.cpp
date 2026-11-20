//
//  ingame_overlay.cpp
//  Moonlight
//
//  Created by Даниил Виноградов on 29.05.2021.
//

#ifdef PLATFORM_SWITCH
#include <borealis/platforms/switch/switch_input.hpp>
#endif

#include "helper.hpp"
#include "MouseScrollSettings.hpp"
#include "SystemButtonSettings.hpp"
#include "ingame_overlay_view.hpp"
#include "streaming_input_overlay.hpp"
#include "button_selecting_dialog.hpp"
#include "UpscalingSupport.hpp"
#include "halfcon_mapping_view.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

#define SET_SETTING(n, func)                                                   \
    case n:                                                                    \
        Settings::instance().func;                                             \
        break;

#define GET_SETTINGS(combo_box, n, i)                                          \
    case n:                                                                    \
        combo_box->setSelection(i);                                            \
        break;

#define DEFAULT                                                                \
    default:                                                                   \
        break;

using namespace brls;

namespace {
void updateStrengthControl(BooleanSliderCell* cell,
                           bool enabled, const std::string& detailText) {
    cell->setSliderVisibility(enabled ? brls::Visibility::VISIBLE
                                      : brls::Visibility::GONE);
    cell->setValueText(enabled ? detailText : "");
}

float sliderProgressToDitheringStrength(float progress) {
    return static_cast<float>(int(progress * 9.0f + 0.5f) + 1);
}

float ditheringStrengthToSliderProgress(float strength) {
    if (strength < 1.0f)
        strength = 1.0f;
    else if (strength > 10.0f)
        strength = 10.0f;

    return (strength - 1.0f) / 9.0f;
}

std::string getDitheringStrengthText(float strength) {
    return std::to_string(int(strength));
}

std::string getRcasStrengthText(float strength) {
    return std::to_string(int(strength * 100.0f)) + "%";
}
}

bool debug = false;

// MARK: - Ingame Overlay View
IngameOverlay::IngameOverlay(StreamingView* streamView)
    : streamView(streamView) {
    brls::Application::registerXMLView(
        "LogoutTab", [streamView]() { return new LogoutTab(streamView); });
    brls::Application::registerXMLView(
        "OptionsTab", [streamView]() { return new OptionsTab(streamView); });

    this->inflateFromXMLRes("xml/views/ingame_overlay/overlay.xml");

    addGestureRecognizer(
        new TapGestureRecognizer([this](TapGestureStatus status, Sound* sound) {
            if (status.state == GestureState::END)
                this->dismiss();
        }));

    applet->addGestureRecognizer(new TapGestureRecognizer(
        [](TapGestureStatus status, Sound* sound) {}));

    getAppletFrameItem()->title =
        streamView->getHost().hostname + ": " + streamView->getApp().name;
    updateAppletFrameItem();
}

brls::AppletFrame* IngameOverlay::getAppletFrame() { return applet; }

// MARK: - Logout Tab
LogoutTab::LogoutTab(StreamingView* streamView) : streamView(streamView) {
    this->inflateFromXMLRes("xml/views/ingame_overlay/logout_tab.xml");

    disconnect->setText("streaming/disconnect"_i18n);
    disconnect->registerClickAction([this, streamView](View* view) {
        this->dismiss([streamView] { streamView->terminate(false); });
        return true;
    });

    terminateButton->setText("streaming/terminate"_i18n);
    terminateButton->registerClickAction([this, streamView](View* view) {
        this->dismiss([streamView] { streamView->terminate(true); });
        return true;
    });
}

// MARK: - Options Tab
OptionsTab::OptionsTab(StreamingView* streamView) : streamView(streamView) {
    this->inflateFromXMLRes("xml/views/ingame_overlay/options_tab.xml");

    guideKeyButtons->setText("settings/guide_key_buttons"_i18n);
    setupButtonsSelectorCell(guideKeyButtons,
                             Settings::instance().guide_key_options().buttons);
    guideKeyButtons->registerClickAction([this](View* view) {
        ButtonSelectingDialog* dialog = ButtonSelectingDialog::create(
            "settings/guide_key_setup_message"_i18n, [this](auto buttons) {
                auto options = Settings::instance().guide_key_options();
                options.buttons = buttons;
                Settings::instance().set_guide_key_options(options);
                setupButtonsSelectorCell(guideKeyButtons, buttons);
            });

        dialog->open();
        return true;
    });

#ifndef PLATFORM_SWITCH
    guideBySystemButton->removeFromSuperView();
#else
    guideBySystemButton->init(
        "settings/use_system_button"_i18n,
        systemButtonOptions(),
        (int) Settings::instance().get_guide_system_button(), [this](int value) {
            if (value != 0 && Settings::instance().get_overlay_system_button() == (ButtonOverrideType) value) {
                brls::sync([this, value](){
                    showError("settings/system_button_duplication_error"_i18n, [](){});
                });
                guideBySystemButton->setSelection((int) Settings::instance().get_guide_system_button(), true);
                return;
            }

            Settings::instance().set_guide_system_button((ButtonOverrideType) value);

            auto color = Settings::instance().get_guide_system_button() == ButtonOverrideType::NONE ?
                Application::getTheme()["brls/text_disabled"] : Application::getTheme()["brls/accent"];
            guideBySystemButton->setDetailTextColor(color);
        });
    auto color = Settings::instance().get_guide_system_button() == ButtonOverrideType::NONE ?
         Application::getTheme()["brls/text_disabled"] : Application::getTheme()["brls/accent"];
    guideBySystemButton->setDetailTextColor(color);
#endif

    volumeHeader->setSubtitle(
        std::to_string(Settings::instance().get_volume()) + "%");
    float amplification =
        Settings::instance().get_volume_amplification() ? 500.0f : 100.0f;
    float progress = (float) Settings::instance().get_volume() / amplification;
    volumeSlider->getProgressEvent()->subscribe(
        [this, amplification](float progress) {
            int volume = int(progress * amplification);
            Settings::instance().set_volume(volume);
            volumeHeader->setSubtitle(std::to_string(volume) + "%");
        });
    volumeSlider->setProgress(progress);

    float rumbleForceProgress = Settings::instance().get_rumble_force();
    rumbleForceSlider->getProgressEvent()->subscribe([this](float value) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << int(value * 100);
        rumbleForceHeader->setSubtitle(stream.str() + "%");
        Settings::instance().set_rumble_force(value);
    });
    rumbleForceSlider->setProgress(rumbleForceProgress);

    float mouseProgress =
        ((float) Settings::instance().get_mouse_speed_multiplier() / 100.0f);
    mouseSlider->getProgressEvent()->subscribe([this](float value) {
        float multiplier = value * 1.5f + 0.5f;
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << multiplier;
        mouseHeader->setSubtitle("x" + stream.str());
        Settings::instance().set_mouse_speed_multiplier(int(value * 100));
    });
    mouseSlider->setProgress(mouseProgress);
    setupMouseScrollSettings(mouseScrollHeader, mouseScrollSlider);

    inputOverlayButton->setText("streaming/mouse_input"_i18n);
    inputOverlayButton->registerClickAction([this](View* view) {
        this->dismiss([this]() {
            auto* overlay =
                new StreamingInputOverlay(this->streamView);
            Application::pushActivity(new Activity(overlay));
        });
        return true;
    });

    std::vector<std::string> keyboardTypes = {
        "settings/keyboard_compact"_i18n, "settings/keyboard_fullsized"_i18n};
    keyboardType->setText("settings/keyboard_type"_i18n);
    keyboardType->setData(keyboardTypes);
    switch (Settings::instance().get_keyboard_type()) {
        GET_SETTINGS(keyboardType, COMPACT, 0)
        GET_SETTINGS(keyboardType, FULLSIZED, 1)
        DEFAULT
    }
    keyboardType->getEvent()->subscribe([](int selected) {
        switch (selected) {
            SET_SETTING(0, set_keyboard_type(COMPACT))
            SET_SETTING(1, set_keyboard_type(FULLSIZED))
            DEFAULT
        }
    });

    std::vector<std::string> keyboardFingersOptions = {
        "3", "4", "5", "hints/off"_i18n};
    keyboardFingers->setText("settings/keyboard_fingers"_i18n);
    keyboardFingers->setData(keyboardFingersOptions);
    switch (Settings::instance().get_keyboard_fingers()) {
        GET_SETTINGS(keyboardFingers, 3, 0)
        GET_SETTINGS(keyboardFingers, 4, 1)
        GET_SETTINGS(keyboardFingers, 5, 2)
        GET_SETTINGS(keyboardFingers, -1, 3)
        DEFAULT
    }
    keyboardFingers->getEvent()->subscribe([](int selected) {
        switch (selected) {
            SET_SETTING(0, set_keyboard_fingers(3))
            SET_SETTING(1, set_keyboard_fingers(4))
            SET_SETTING(2, set_keyboard_fingers(5))
            SET_SETTING(3, set_keyboard_fingers(-1))
            DEFAULT
        }
    });

    touchscreenMouseMode->init("settings/touchscreen_mouse_mode"_i18n,
                               Settings::instance().touchscreen_mouse_mode(),
                               [](bool value) {
                                   Settings::instance().set_touchscreen_mouse_mode(value);
                               });
    
    swapStickToDpad->init("settings/swap_stick_to_dpad"_i18n, Settings::instance().swap_joycon_stick_to_dpad(),
                          [](bool value) { Settings::instance().set_swap_joycon_stick_to_dpad(value); });

#ifdef PLATFORM_SWITCH
    auto* joyconInput = static_cast<brls::SwitchInputManager*>(Application::getPlatform()->getInputManager());
    joyconInputMode->setText("settings/joycon_input_mode"_i18n);
    joyconInputMode->setData({
        "settings/joycon_input_separate"_i18n,
        "settings/joycon_input_paired"_i18n
    });
    joyconInputMode->setSelection(Settings::instance().joycon_paired_mode() ? 1 : 0);
    joyconInputMode->getEvent()->subscribe([](int selected) {
        Settings::instance().set_joycon_paired_mode(selected == 1);
        Settings::instance().save();
    });

    joyconPairGyroMode->setText("settings/joycon_pair_gyro_mode"_i18n);
    joyconPairGyroMode->setData({
        "settings/joycon_pair_gyro_right"_i18n,
        "settings/joycon_pair_gyro_left"_i18n,
        "settings/joycon_pair_gyro_both"_i18n
    });
    joyconPairGyroMode->setSelection(Settings::instance().joycon_pair_gyro_mode());
    joyconPairGyroMode->getEvent()->subscribe([](int selected) {
        Settings::instance().set_joycon_pair_gyro_mode(selected);
        Settings::instance().save();
    });

    joyconDsu->init("settings/joycon_dsu"_i18n, Settings::instance().joycon_dsu_enabled(),
        [](bool enabled) {
            if (!Settings::instance().set_joycon_dsu_enabled(enabled))
                Application::notify("settings/joycon_dsu_failed"_i18n);
            Settings::instance().save();
        });

    joyconCalibrate->setText("settings/joycon_calibrate"_i18n);
    joyconCalibrate->setDetailText("settings/joycon_calibrate_instructions"_i18n);
    joyconCalibrate->registerClickAction([this](View*) {
        auto* input = static_cast<brls::SwitchInputManager*>(Application::getPlatform()->getInputManager());
        const bool ok = input->calibrateJoyConsOnFlatSurface();
        if (ok)
            Settings::instance().save();
        joyconCalibrate->setDetailText(ok ? "settings/joycon_calibrate_success"_i18n
                                        : "settings/joycon_calibrate_failed"_i18n);
        Application::notify(ok ? "settings/joycon_calibrate_success"_i18n
                               : "settings/joycon_calibrate_failed"_i18n);
        return true;
    });

    joyconMapping->setText("halfcon/title"_i18n);
    joyconMapping->registerClickAction([this](View*) {
        this->present(new HalfConMappingView());
        return true;
    });

    joyconDsuStatus->setText("settings/joycon_dsu_status"_i18n);
    auto updateDsuStatus = [this, joyconInput]() {
        if (!joyconInput->isDsuMotionEnabled())
        {
            joyconDsuStatus->setDetailText("settings/joycon_dsu_off"_i18n);
            return;
        }
        const std::string sensors =
            std::string(joyconInput->isDsuMotionSensorPresent(brls::HalfJoyConSide::LEFT) ? "L" : "-")
            + (joyconInput->isDsuMotionSensorPresent(brls::HalfJoyConSide::RIGHT) ? " R" : " -");
        joyconDsuStatus->setDetailText("Peers: " + std::to_string(joyconInput->getDsuMotionSubscribers())
            + " | IMU: " + sensors
            + " | RX: " + std::to_string(joyconInput->getDsuReceivedRequests())
            + " | TX: " + std::to_string(joyconInput->getDsuSentPackets())
            + " | ERR: " + std::to_string(joyconInput->getDsuLastError()));
    };
    updateDsuStatus();
    joyconDsuStatus->registerClickAction([updateDsuStatus](View*) {
        updateDsuStatus();
        return true;
    });
#else
    joyconInputMode->setVisibility(Visibility::GONE);
    joyconPairGyroMode->setVisibility(Visibility::GONE);
    joyconDsu->setVisibility(Visibility::GONE);
    joyconCalibrate->setVisibility(Visibility::GONE);
    joyconDsuStatus->setVisibility(Visibility::GONE);
    joyconMapping->setVisibility(Visibility::GONE);
#endif

    onscreenLogButton->init("streaming/show_logs"_i18n,
                            Settings::instance().write_log(), [](bool value) {
                                Settings::instance().set_write_log(value);
                                brls::Application::enableDebuggingView(value);
                            });

    debugButton->init(
        "streaming/debug_info"_i18n, streamView->draw_stats,
        [streamView](bool value) { streamView->draw_stats = value; });

#ifdef SUPPORT_UPSCALING
    if (!isVideoUpscalingSupported()) {
        imageAdjustmentsHeader->removeFromSuperView(true);
        ditheringButton->removeFromSuperView(true);
        upscalingButton->removeFromSuperView(true);
        upscalingModeButton->removeFromSuperView(true);
        rcasButton->removeFromSuperView(true);
    } else {
        auto updateDitheringControls = [this](bool enabled) {
            updateStrengthControl(ditheringButton, enabled,
                                  getDitheringStrengthText(
                                      Settings::instance().dithering_strength()));
        };
        auto updateRcasControls = [this](bool enabled) {
            updateStrengthControl(rcasButton, enabled,
                                  getRcasStrengthText(
                                      Settings::instance().rcas_strength()));
        };

        ditheringButton->init(
            "settings/dithering"_i18n, Settings::instance().dithering(),
            [updateDitheringControls](bool value) {
                Settings::instance().set_dithering(value);
                updateDitheringControls(value);
            });

        const float ditheringStrength = Settings::instance().dithering_strength();
        ditheringButton->getProgressEvent()->subscribe([this](float value) {
            const float strength = sliderProgressToDitheringStrength(value);
            Settings::instance().set_dithering_strength(strength);
            updateStrengthControl(ditheringButton,
                                  Settings::instance().dithering(),
                                  getDitheringStrengthText(strength));
        });
        ditheringButton->setProgress(
            ditheringStrengthToSliderProgress(ditheringStrength));
        updateDitheringControls(Settings::instance().dithering());

#if defined(PLATFORM_APPLE) && !defined(PLATFORM_TVOS)
        upscalingButton->removeFromSuperView(true);
        upscalingModeButton->init(
            "settings/upscaling"_i18n,
            {"hints/off"_i18n, "MetalFX", "FSR1"},
            (int)Settings::instance().upscaling_mode(),
            [](int value) { Settings::instance().set_upscaling_mode((UpscalingMode)value); });
#else
        upscalingModeButton->removeFromSuperView(true);
        upscalingButton->init(
            "settings/upscaling"_i18n, Settings::instance().upscaling(),
            [](bool value) { Settings::instance().set_upscaling(value); });
#endif
        rcasButton->init(
            "settings/rcas_sharpening"_i18n, Settings::instance().rcas(),
            [updateRcasControls](bool value) {
                Settings::instance().set_rcas(value);
                updateRcasControls(value);
            });

        const float rcasStrength = Settings::instance().rcas_strength();
        rcasButton->getProgressEvent()->subscribe([this](float value) {
            Settings::instance().set_rcas_strength(value);
            updateStrengthControl(rcasButton,
                                  Settings::instance().rcas(),
                                  getRcasStrengthText(value));
        });
        rcasButton->setProgress(rcasStrength);
        updateRcasControls(Settings::instance().rcas());
    }
#else
    imageAdjustmentsHeader->removeFromSuperView(true);
    ditheringButton->removeFromSuperView(true);
    upscalingButton->removeFromSuperView(true);
    upscalingModeButton->removeFromSuperView(true);
    rcasButton->removeFromSuperView(true);
#endif
}

OptionsTab::~OptionsTab() { Settings::instance().save(); }

std::string
OptionsTab::getTextFromButtons(std::vector<ControllerButton> buttons) {
    std::string buttonsText;
    if (!buttons.empty()) {
        for (int i = 0; i < buttons.size(); i++) {
            buttonsText += brls::Hint::getKeyIcon(buttons[i], true);
            if (i < buttons.size() - 1)
                buttonsText += " + ";
        }
    } else {
        buttonsText = "hints/off"_i18n;
    }
    return buttonsText;
}

NVGcolor
OptionsTab::getColorFromButtons(const std::vector<brls::ControllerButton>& buttons) {
    Theme theme = Application::getTheme();
    return buttons.empty() ? theme["brls/text_disabled"]
                           : theme["brls/list/listItem_value_color"];
}

void OptionsTab::setupButtonsSelectorCell(brls::DetailCell* cell, const std::vector<ControllerButton>& buttons) {
    cell->setDetailText(getTextFromButtons(buttons));
    cell->setDetailTextColor(getColorFromButtons(buttons));
}
