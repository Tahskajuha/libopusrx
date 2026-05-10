MSON := builds/meson_compiled
JNI_LIBS := android_demo/receiver/src/main/jniLibs/arm64-v8a
CONFIG_DIR := /etc/opusrx
SYSTEMD_DIR := /etc/systemd/system
CURRENT_USER := $(shell whoami)
CURRENT_UID := $(shell id -u)

.PHONY: meson meson-setup install-native-demo gradle all clean

all: meson gradle

meson-setup:
	meson setup --wipe $(MSON)
	ln -sfn $(MSON)/compile_commands.json compile_commands.json

meson:
	meson setup --wipe $(MSON)
	ninja -C $(MSON)

install-native-demo:
	meson setup --wipe $(MSON) -Dinstall_demo=true
	ninja -C $(MSON)
	sudo meson install -C $(MSON)
	sudo mkdir -p $(CONFIG_DIR)
	if [ ! -f "$(CONFIG_DIR)/opusrx.conf" ]; then \
		sudo install -Dm644 native_demo/dist/opusrx.conf $(CONFIG_DIR)/opusrx.conf; \
	fi
	sed -e 's/@USER@/$(CURRENT_USER)/g' -e 's/@UID@/$(CURRENT_UID)/g' native_demo/dist/opusrxd.service.in > /tmp/opusrxd.service
	sudo install -Dm644 /tmp/opusrxd.service $(SYSTEMD_DIR)/opusrxd.service
	sudo systemctl daemon-reload
	rm -f /tmp/opusrxd.service
	@echo ""
	@echo "Installed opusrx and opusrxd service."
	@echo "Edit config:"
	@echo "   $(CONFIG_DIR)/opusrx.conf"
	@echo ""
	@echo "Enable service with:"
	@echo "   sudo systemctl enable --now opusrxd"

gradle:
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
