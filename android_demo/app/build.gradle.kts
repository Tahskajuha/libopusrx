val appName = "opusrx"
val packageName = "com.example.opusrx"

plugins {
  id("com.android.application")
  id("org.jetbrains.kotlin.android")
}

layout.buildDirectory = rootProject.layout.buildDirectory.dir("app")

android {
  namespace = packageName
  compileSdk = 36

  defaultConfig {
    applicationId = packageName
    minSdk = 24
    targetSdk = 36
    versionCode = 1
    versionName = "1.0"
  }

  buildTypes {
    release {
      isMinifyEnabled = false
    }
  }

  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_21
    targetCompatibility = JavaVersion.VERSION_21
  }

  kotlin {
	jvmToolchain {
	  languageVersion.set(JavaLanguageVersion.of(21))
	}
  }
}

dependencies {
  implementation("androidx.core:core-ktx:1.17.0")
  implementation("androidx.appcompat:appcompat:1.7.1")
}
