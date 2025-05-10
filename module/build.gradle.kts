plugins {
    id("com.android.application")
}

val verCode = 3
val verName = "v1.0.0"
val pkgName = "top.yukonga.fixXiaomiBoldFont"

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
            abiFilters.add("armeabi-v7a")
            abiFilters.add("arm64-v8a")
        }
    }

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
    archiveFileName.set("FixXiaomiBoldFont_${verName}.zip")
}

afterEvaluate {
    tasks["assembleModule"].dependsOn(tasks["assembleRelease"])
}
