plugins {
    id("com.android.library")
}

java {
    sourceCompatibility = JavaVersion.VERSION_21
    targetCompatibility = JavaVersion.VERSION_21
}

android {
    namespace = "android"
    compileSdk = 36
    compileSdkMinor = 1
    buildToolsVersion = "36.1.0"
    ndkVersion = "29.0.14206865"
}