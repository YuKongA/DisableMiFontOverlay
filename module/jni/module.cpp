#include <android/log.h>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include "zygisk.hpp"

static constexpr auto TAG = "FixXiaomiBoldFont";

#define LOGD(...)     __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

class FixXiaomiBoldFont : public zygisk::ModuleBase {
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
        jclass fontSettingsClass = env->FindClass("miui/util/font/FontNameUtil");
        if (fontSettingsClass == nullptr) {
            LOGD("Failed to find FontNameUtil class");
            return;
        }

        jfieldID hasCustomFontField = env->GetStaticFieldID(fontSettingsClass,
                                                            "FONT_NAME_MIPRO_MEDIUM",
                                                            "[Ljava/lang/String;");
        if (hasCustomFontField == nullptr) {
            LOGD("Failed to find FONT_NAME_MIPRO_MEDIUM field");
            return;
        }

        jobjectArray boldFontArray = env->NewObjectArray(1, env->FindClass("java/lang/String"),
                                                         env->NewStringUTF("miui-bold"));
        env->SetStaticObjectField(fontSettingsClass, hasCustomFontField, boldFontArray);
    }
};

REGISTER_ZYGISK_MODULE(FixXiaomiBoldFont)
