#define TESLA_INIT_IMPL
#include <exception_wrap.hpp>
#include <tesla.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

constexpr const char* kConfigPath = "sdmc:/config/sys-hdr/config.ini";
constexpr const char* kSection = "tonemap";
constexpr const char* kSectionHandheld = "tonemap_handheld";
constexpr const char* kSectionDocked = "tonemap_docked";

constexpr float kExposureMin = 0.25f, kExposureMax = 4.0f;
constexpr float kWhitePointMin = 1.0f, kWhitePointMax = 16.0f;
constexpr float kExpandMin = 0.0f, kExpandMax = 0.95f;
constexpr float kContrastMin = 0.5f, kContrastMax = 2.0f;

const std::vector<std::string> kCurveNames = {
    "Off", "Reinhard", "Reinhard Ext", "Hable", "ACES",
};

std::string readValue(const char* section, const char* key, const std::string& defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, section, key);
    return raw.empty() ? defVal : raw;
}

bool readBool(const char* section, const char* key, bool defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, section, key);
    if (raw.empty())
        return defVal;
    return raw == "true" || raw == "1";
}

float readFloat(const char* section, const char* key, float defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, section, key);
    if (raw.empty())
        return defVal;
    return std::strtof(raw.c_str(), nullptr);
}

int readCurve(const char* section, int defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, section, "curve");
    if (raw.empty())
        return defVal;
    const int v = std::atoi(raw.c_str());
    return (v >= 0 && v < static_cast<int>(kCurveNames.size())) ? v : defVal;
}

void writeValue(const char* section, const char* key, const std::string& value) {
    ult::setIniFileValue(kConfigPath, section, key, value);
}

void writeBool(const char* section, const char* key, bool value) {
    writeValue(section, key, value ? "true" : "false");
}

void writeFloat(const char* section, const char* key, float value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.3f", value);
    writeValue(section, key, buf);
}

std::string formatFloat(float value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return buf;
}

void addRangeTrackBar(tsl::elm::List* list, const char* section, const std::string& label, const char* key,
                       float lo, float hi, float initialValue) {
    auto* header = new tsl::elm::CategoryHeader(label);
    header->setValue(formatFloat(initialValue));
    list->addItem(header);

    const int initialProgress = static_cast<int>(std::lround((initialValue - lo) / (hi - lo) * 100.0f));

    auto* trackBar = new tsl::elm::TrackBar("");
    trackBar->setProgress(static_cast<u16>(std::clamp(initialProgress, 0, 100)));
    trackBar->setValueChangedListener([section, key, lo, hi, header](u16 progress) {
        const float value = lo + (hi - lo) * (progress / 100.0f);
        writeFloat(section, key, value);
        header->setValue(formatFloat(value));
    });
    list->addItem(trackBar);
}

void addProfileSection(tsl::elm::List* list, const char* section, const std::string& title) {
    list->addItem(new tsl::elm::CategoryHeader(title));

    auto* enabledItem = new tsl::elm::ToggleListItem("Enabled", readBool(section, "enabled", true), "On", "Off");
    enabledItem->setStateChangedListener([section](bool state) {
        writeBool(section, "enabled", state);
    });
    list->addItem(enabledItem);

    const int initialCurve = readCurve(section, 3);
    auto* curveBar = new tsl::elm::NamedStepTrackBar("", {
        kCurveNames[0], kCurveNames[1], kCurveNames[2], kCurveNames[3], kCurveNames[4],
    });
    curveBar->setProgress(static_cast<u16>(initialCurve));
    curveBar->setValueChangedListener([section](u16 index) {
        writeValue(section, "curve", std::to_string(index));
    });
    list->addItem(curveBar);

    addRangeTrackBar(list, section, "Exposure", "exposure", kExposureMin, kExposureMax,
        readFloat(section, "exposure", 1.0f));
    addRangeTrackBar(list, section, "White point", "white_point", kWhitePointMin, kWhitePointMax,
        readFloat(section, "white_point", 4.0f));
    addRangeTrackBar(list, section, "Expand", "expand", kExpandMin, kExpandMax,
        readFloat(section, "expand", 0.0f));
    addRangeTrackBar(list, section, "Contrast", "contrast", kContrastMin, kContrastMax,
        readFloat(section, "contrast", 1.0f));
}

} // namespace

class GuiMain : public tsl::Gui {
  public:
    tsl::elm::Element* createUI() override {
        auto* frame = new tsl::elm::OverlayFrame("sys-HDR", "Tonemap");
        auto* list = new tsl::elm::List();

        list->addItem(new tsl::elm::CategoryHeader("Overlay"));

        auto* globalEnabledItem = new tsl::elm::ToggleListItem("Global enable", readBool(kSection, "enabled", true), "On", "Off");
        globalEnabledItem->setStateChangedListener([](bool state) {
            writeBool(kSection, "enabled", state);
        });
        list->addItem(globalEnabledItem);

        addProfileSection(list, kSectionHandheld, "Handheld");
        addProfileSection(list, kSectionDocked, "Docked");

        frame->setContent(list);
        return frame;
    }
};

class ToneMapOverlay : public tsl::Overlay {
  public:
    void initServices() override {}
    void exitServices() override {}

    void onShow() override {}
    void onHide() override {}

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiMain>();
    }
};

int main(int argc, char** argv) {
    return tsl::loop<ToneMapOverlay>(argc, argv);
}
