plugins {
  id("com.android.application") version "8.12.1" apply false
  id("org.jetbrains.kotlin.android") version "2.2.0" apply false
}
layout.buildDirectory = layout.projectDirectory.dir("../builds/gradle_compiled")
tasks.register("clean", Delete::class) {
  delete(rootProject.layout.buildDirectory)
}
