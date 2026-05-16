# An RTP Audio Receiver and Playback Library

OpusRX is a lightweight cross-platform RTP audio receiver library designed for controllable latency/jitter and real-time playback, focusing exclusively on RTP reception, buffering, decoding, and playback orchestration rather than full VoIP/session management.

## Installation

The core library can be installed using Meson:

```
meson setup -Dinstall_jni=true build/
ninja -C build/
sudo meson install -C build/
```

Note that `install_jni` is disabled by default. Without enabling it, only the core library will be installed.

The repository also includes:

- a native ALSA-based demo/service for Linux
- an Android demo application using the JNI wrapper

The native demo/service can be installed using Make:

```
make install-native-demo
make uninstall-native-demo
```

The Android demo can also be built using Make:

```
make all
```

This will compile the Android app in debug mode. The apk will be generated at `./builds/gradle_compiled/app/outputs/apk/debug/app-debug.apk`.

## Features

- **User-configurable Latency and Jitter Tolerance**: User decides the buffer size, target buffer depth and the tolerance to be allowed when maintaining the target depth. The library does not impose a fixed buffering policy.

- **Reusable pipe-style architecture**: Transport-agnostic RTP packet ingestion and pull-based playback API allow the library to easily integrate with any PCM consumer and any RTP source carrying OPUS payloads.

- **PLC/FEC Support**: Designed to always return a frame whenever called, the player uses PLC/FEC techniques effectively to ensure that the target buffer depth is maintained.

- **Persistent Operation**: Intended for unattended operation, the player automatically idles when packets stop arriving for longer than the configured timeout.

- **Thread Safe**: Queue-based packet ingestion that can run asynchronous to the packet decoder, allowing separation of network and playback clocks.

## API Reference

> Current WIP as the API is going through some rapid changes. This part will be added soon.
