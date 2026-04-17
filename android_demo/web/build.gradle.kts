plugins {
  id("com.android.library")
  id("org.jetbrains.kotlin.android")
}

layout.buildDirectory = rootProject.layout.buildDirectory.dir("web")

android {
  namespace = "com.example.opusrx.web"
  compileSdk = 36

  defaultConfig {
    minSdk = 24
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
