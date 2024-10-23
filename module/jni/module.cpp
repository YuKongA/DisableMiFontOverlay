#include <android/log.h>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include "zygisk.hpp"

#include "zygisk.hpp"

static constexpr auto TAG = "DisableMiFontOverlay";

#define LOGD(...)     __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#define CLASSES_DEX "/data/adb/modules/DisableMiFontOverlay/classes.dex"

ssize_t xread(int fd, void *buffer, size_t count) {
    ssize_t total = 0;
    char *buf = (char *) buffer;
    while (count > 0) {
        ssize_t ret = read(fd, buf, count);
        if (ret < 0) return -1;
        buf += ret;
        total += ret;
        count -= ret;
    }
    return total;
}

ssize_t xwrite(int fd, void *buffer, size_t count) {
    ssize_t total = 0;
    char *buf = (char *) buffer;
    while (count > 0) {
        ssize_t ret = write(fd, buf, count);
        if (ret < 0) return -1;
        buf += ret;
        total += ret;
        count -= ret;
    }
    return total;
}

std::vector<std::string> split(const std::string &strTotal) {
    std::vector<std::string> vecResult;
    std::istringstream iss(strTotal);
    std::string token;

    while (std::getline(iss, token, '\n')) {
        vecResult.push_back("/" + token);
    }

    return std::move(vecResult);
}

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


        int fd = api->connectCompanion();

        long dexSize = 0;

        xread(fd, &dexSize, sizeof(long));

        LOGD("Dex file size: %ld", dexSize);

        if (dexSize < 1) {
            close(fd);
            return;
        }

        dexVector.resize(dexSize);
        xread(fd, dexVector.data(), dexSize);
        close(fd);
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (dexVector.empty()) return;
        injectDex();
    }

    void preServerSpecialize(zygisk::ServerSpecializeArgs *args) override {
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    std::vector<uint8_t> dexVector;

    void injectDex() {
        auto clClass = env->FindClass("java/lang/ClassLoader");
        auto getSystemClassLoader = env->GetStaticMethodID(clClass, "getSystemClassLoader",
                                                           "()Ljava/lang/ClassLoader;");
        auto systemClassLoader = env->CallStaticObjectMethod(clClass, getSystemClassLoader);

        auto dexClClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
        auto dexClInit = env->GetMethodID(dexClClass, "<init>",
                                          "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
        auto buffer = env->NewDirectByteBuffer(dexVector.data(), dexVector.size());
        auto dexCl = env->NewObject(dexClClass, dexClInit, buffer, systemClassLoader);

        auto loadClass = env->GetMethodID(clClass, "loadClass",
                                          "(Ljava/lang/String;)Ljava/lang/Class;");
        auto entryClassName = env->NewStringUTF("top.yukonga.disableMiFontOverlay.Main");
        auto entryClassObj = env->CallObjectMethod(dexCl, loadClass, entryClassName);

        auto entryPointClass = (jclass) entryClassObj;

        auto entryInit = env->GetStaticMethodID(entryPointClass, "init", "()V");
        env->CallStaticVoidMethod(entryPointClass, entryInit);
    }
};

static std::vector<uint8_t> readFile(const char *path) {

    std::vector<uint8_t> vector;

    FILE *file = fopen(path, "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        vector.resize(size);
        fread(vector.data(), 1, size, file);
        fclose(file);
    } else {
        LOGD("Couldn't read %s file!", path);
    }

    return vector;
}

static void companion(int fd) {
    std::vector<uint8_t> dexVector;

    dexVector = readFile(CLASSES_DEX);

    long dexSize = dexVector.size();

    xwrite(fd, &dexSize, sizeof(long));
    xwrite(fd, dexVector.data(), dexSize);
}

// Register our module class and the companion handler function
REGISTER_ZYGISK_MODULE(DisableMiFontOverlay)

REGISTER_ZYGISK_COMPANION(companion)