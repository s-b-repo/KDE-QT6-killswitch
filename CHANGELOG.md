# Changelog

All notable changes to KDE-QT6-killswitch are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- Polkit policy file (`org.kde.killswitch.policy`) so the existing rules file
  actually takes effect with pkexec. Without it, pkexec ignored the custom
  action ID and always prompted for a password.
- Consecutive-failure threshold before shutdown. The kill switch now requires
  3 back-to-back failed connectivity checks before triggering, preventing
  spurious shutdowns from momentary network hiccups (Wi-Fi roaming, DHCP
  renewal, brief packet loss).
- Fallback to `poweroff` if the `shutdown` command fails.
- Error reporting when `pkexec` or `shutdown` invocations fail (previously
  silent).

### Fixed

- **Build: wrong install target name.** `install(TARGETS LinuxKillSwitch ...)`
  referenced a target that did not exist. Changed to match the actual target
  `KDE-QT6-killswitch`.
- **Build: missing `CMAKE_AUTOMOC`.** The source includes `main.moc` but CMake
  was never told to run the meta-object compiler. Added `set(CMAKE_AUTOMOC ON)`.
- **Build: missing headers.** Added `#include <QCloseEvent>` and
  `#include <QIcon>` which were used but never included. Compilation could fail
  depending on toolchain.
- **Runtime: `QCoreApplication::applicationFilePath()` called before any
  `QApplication` existed.** This returned an empty string on most systems,
  silently breaking the pkexec re-launch. Replaced with `argv[0]`.
- **Runtime: blocking TCP connections on the GUI thread.** Two sequential
  `waitForConnected()` calls could freeze the UI for up to 6 seconds every
  check cycle. Moved connectivity checks to a thread pool via
  `QtConcurrent::run`.
- **Runtime: `size_t` to `quint32` implicit narrowing** in
  `QRandomGenerator::bounded()` calls. Added explicit casts.
- **Runtime: `closeEvent` override did not call base class** when the tray icon
  was not visible, skipping `QMainWindow` cleanup.
