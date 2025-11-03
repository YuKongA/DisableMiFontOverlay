plugins {
    id("com.android.application")
}

val verCode = 5
val verName = "v1.0.5"
val pkgName = "top.yukonga.disableMiFontOverlay"

java {
    sourceCompatibility = JavaVersion.VERSION_21
    targetCompatibility = JavaVersion.VERSION_21
}

android {
    compileSdk = 36
    namespace = pkgName
    externalNativeBuild {
        ndkBuild {
            path = file("jni/Android.mk")
        }
    }
    defaultConfig {
        applicationId = pkgName
        minSdk = 33
        targetSdk = 36
        versionCode = verCode
        versionName = verName
        ndk {
            abiFilters.addAll(mutableSetOf("arm64-v8a", "armeabi-v7a"))
        }
    }
    compileSdkMinor = 1
    buildToolsVersion = "36.1.0"
    ndkVersion = "29.0.14206865"
}

dependencies {
    compileOnly(project(":hiddenapi"))
}

tasks.register<Zip>("assembleModule") {
    group = "module"
    val zipTree = zipTree(layout.buildDirectory.file("outputs/apk/release/module-release-unsigned.apk").get().asFile)
    from(zipTree) {
        include("assets/**", "lib/**")
        exclude("assets/module.prop")
        eachFile {
            path = when {
                path.startsWith("lib/") -> buildString {
                    val startIndex = path.indexOf('/') + 1
                    val endIndex = path.indexOf('/', startIndex)
                    append("zygisk/")
                    append(path.substring(startIndex, endIndex))
                    append(".so")
                }

                path.startsWith("assets/") -> path.replace("assets/", "")

                else -> path
            }
        }
    }
    from(file("src/main/assets/module.prop")) {
        filter { line ->
            line.replace("%%VERSION%%", verName)
                .replace("%%VERSIONCODE%%", verCode.toString())
        }
    }
    destinationDirectory.set(layout.buildDirectory.dir("outputs/module"))
    archiveFileName.set("DisableMiFontOverlay_${verName}.zip")
}

afterEvaluate {
    tasks["assembleModule"].dependsOn(tasks["assembleRelease"])
}
