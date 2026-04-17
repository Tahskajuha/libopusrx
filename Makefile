MSON := builds/meson_compiled
JNI_LIBS := android_demo/receiver/src/main/jniLibs/arm64-v8a

.PHONY: native android all clean

all: native android

native-setup:
	meson setup --wipe $(MSON)
	ln -sfn $(MSON)/compile_commands.json compile_commands.json

native:
	meson setup --wipe $(MSON)
	ninja -C $(MSON)

android:
	@if [ -f "$(MSON)/jni/libopusrx-jni.so" ]; then \
		cp $(MSON)/jni/libopusrx-jni.so $(JNI_LIBS)/; \
		cd android_demo && ./gradlew assembleDebug; \
		cd ..; \
	else \
		echo "Build the jni library first!"; \
	fi

clean:
	rm -rf builds
	rm compile_commands.json
