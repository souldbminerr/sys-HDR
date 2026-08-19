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

constexpr float kExposureMin = 0.25f, kExposureMax = 4.0f;
constexpr float kWhitePointMin = 1.0f, kWhitePointMax = 16.0f;
constexpr float kExpandMin = 0.0f, kExpandMax = 0.95f;
constexpr float kContrastMin = 0.5f, kContrastMax = 2.0f;

const std::vector<std::string> kCurveNames = {
    "Off", "Reinhard", "Reinhard Ext", "Hable", "ACES",
};

std::string readValue(const char* key, const std::string& defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, kSection, key);
    return raw.empty() ? defVal : raw;
}

bool readBool(const char* key, bool defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, kSection, key);
    if (raw.empty())
        return defVal;
    return raw == "true" || raw == "1";
}

float readFloat(const char* key, float defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, kSection, key);
    if (raw.empty())
        return defVal;
    return std::strtof(raw.c_str(), nullptr);
}

int readCurve(int defVal) {
    const std::string raw = ult::parseValueFromIniSection(kConfigPath, kSection, "curve");
    if (raw.empty())
        return defVal;
    const int v = std::atoi(raw.c_str());
    return (v >= 0 && v < static_cast<int>(kCurveNames.size())) ? v : defVal;
}

void writeValue(const char* key, const std::string& value) {
    ult::setIniFileValue(kConfigPath, kSection, key, value);
}

void writeBool(const char* key, bool value) {
    writeValue(key, value ? "true" : "false");
}

void writeFloat(const char* key, float value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.3f", value);
    writeValue(key, buf);
}

void addRangeTrackBar(tsl::elm::List* list, const std::string& label, const char* key,
                       float lo, float hi, float initialValue) {
    list->addItem(new tsl::elm::CategoryHeader(label));

    const int initialProgress = static_cast<int>(std::lround((initialValue - lo) / (hi - lo) * 100.0f));

    auto* trackBar = new tsl::elm::TrackBar("");
    trackBar->setProgress(static_cast<u16>(std::clamp(initialProgress, 0, 100)));
    trackBar->setValueChangedListener([key, lo, hi](u16 progress) {
        writeFloat(key, lo + (hi - lo) * (progress / 100.0f));
    });
    list->addItem(trackBar);
}

} // namespace

class GuiMain : public tsl::Gui {
  public:
    tsl::elm::Element* createUI() override {
        auto* frame = new tsl::elm::OverlayFrame("sys-HDR", "Tonemap");
        auto* list = new tsl::elm::List();

        list->addItem(new tsl::elm::CategoryHeader("Overlay"));

        auto* enabledItem = new tsl::elm::ToggleListItem("Enabled", readBool("enabled", true), "On", "Off");
        enabledItem->setStateChangedListener([](bool state) {
            writeBool("enabled", state);
        });
        list->addItem(enabledItem);

        list->addItem(new tsl::elm::CategoryHeader("Curve"));
        auto* curveBar = new tsl::elm::NamedStepTrackBar("", {
            kCurveNames[0], kCurveNames[1], kCurveNames[2], kCurveNames[3], kCurveNames[4],
        });
        curveBar->setProgress(static_cast<u16>(readCurve(3)));
        curveBar->setValueChangedListener([](u16 index) {
            writeValue("curve", std::to_string(index));
        });
        list->addItem(curveBar);

        addRangeTrackBar(list, "Exposure", "exposure", kExposureMin, kExposureMax,
            readFloat("exposure", 1.0f));
        addRangeTrackBar(list, "White point", "white_point", kWhitePointMin, kWhitePointMax,
            readFloat("white_point", 4.0f));
        addRangeTrackBar(list, "Expand", "expand", kExpandMin, kExpandMax,
            readFloat("expand", 0.0f));
        addRangeTrackBar(list, "Contrast", "contrast", kContrastMin, kContrastMax,
            readFloat("contrast", 1.0f));

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
