#include <android/log.h>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include "zygisk.hpp"

#include "zygisk.hpp"

static constexpr auto TAG = "DisableMiFontOverlay";

#define LOGD(...)     __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

class DisableMiFontOverlay : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *pApi, JNIEnv *pEnv) override {
        this->api = pApi;
        this->env = pEnv;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);

        if (!args) return;

        const char *rawDir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        if (!rawDir) return;

        std::string dir(rawDir);
        env->ReleaseStringUTFChars(args->app_data_dir, rawDir);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        injectDex();
    }

    void preServerSpecialize(zygisk::ServerSpecializeArgs *args) override {
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;

    void injectDex() {
        jclass fontSettingsClass = env->FindClass("miui/util/font/FontSettings");
        if (fontSettingsClass == nullptr) {
            LOGD("Failed to find FontSettings class");
            return;
        }

        jfieldID hasCustomFontField = env->GetStaticFieldID(fontSettingsClass, "HAS_MIUI_VAR_FONT","Z");
        if (hasCustomFontField == nullptr) {
            LOGD("Failed to find HAS_MIUI_VAR_FONT field");
            return;
        }

        env->SetStaticBooleanField(fontSettingsClass, hasCustomFontField, JNI_FALSE);
        LOGD("Successfully set HAS_MIUI_VAR_FONT to false");
    }
};

REGISTER_ZYGISK_MODULE(DisableMiFontOverlay)
