plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val verCode = 1
val verName = "v1.0"
val pkgName = "top.yukonga.disableMiFontOverlay"


android {
    compileSdk = 35
    namespace = pkgName

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = JavaVersion.VERSION_17.toString()
    }

    externalNativeBuild {
        ndkBuild {
            path = file("jni/Android.mk")
        }
    }

    defaultConfig {
        applicationId = pkgName
        minSdk = 33
        targetSdk = 35
        versionCode = verCode
        versionName = verName
    }

    buildTypes {
        named("release") {
            isShrinkResources = true
            isMinifyEnabled = true
            proguardFiles("proguard-rules.pro")
        }
    }
}

dependencies {
    compileOnly(project(":hiddenapi"))
}

tasks.register<Zip>("assembleModule") {
    val zipTree = zipTree(layout.buildDirectory.file("outputs/apk/release/module-release-unsigned.apk").get().asFile)
    from(zipTree) {
        include("assets/**", "lib/**", "classes.dex")
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
    archiveFileName.set("DisableMiFontOverlay.zip")
}

afterEvaluate {
    tasks["assembleModule"].dependsOn(tasks["assembleRelease"])
}