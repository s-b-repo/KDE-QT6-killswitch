# KDE-QT6-killswitch

A Qt6-based connectivity kill switch for Linux. Monitors your internet
connection in real time and triggers an immediate system shutdown when
connectivity is lost, protecting against unintended network exposure.

## How it works

When the kill switch is enabled the daemon runs a connectivity check every
5-10 seconds (randomised). Each check:

1. Verifies that at least one non-loopback network interface is up.
2. Attempts a TCP connection to a randomly chosen DNS server (port 53).
3. Attempts a TCP connection to a randomly chosen website (port 80).

If both the DNS and website checks fail for **3 consecutive cycles**, and no
blocking conditions are active, the daemon calls `shutdown -h now` (falling
back to `poweroff` if that fails).

Connectivity checks run in a background thread so the GUI never blocks.

### Blocking conditions

Two checkboxes let you prevent shutdown while critical work is in progress:

- **File Operation in Progress** — e.g. large transfers.
- **System Upgrade/Installation in Progress** — e.g. package manager running.

While either is checked, failed connectivity is logged but shutdown is deferred.

## Dependencies

- **Qt 6** — Widgets, Network, and Concurrent modules.
- **polkit / pkexec** — for privilege elevation.
- **Linux** — uses `shutdown`/`poweroff` and `geteuid()`.

On Debian/Ubuntu:

```
sudo apt install qt6-base-dev libqt6concurrent6 policykit-1
```

On Fedora:

```
sudo dnf install qt6-qtbase-devel polkit
```

On Arch:

```
sudo pacman -S qt6-base polkit
```

## Building

```sh
git clone https://github.com/s-b-repo/KDE-QT6-killswitch.git
cd KDE-QT6-killswitch
mkdir build && cd build
cmake ..
make
```

The resulting binary is `build/KDE-QT6-killswitch`.

## Installation

From the build directory:

```sh
sudo make install
```

This installs:

| File | Destination |
|------|-------------|
| `KDE-QT6-killswitch` | `/usr/local/bin/` |
| `resources/` | `/usr/local/share/KDE-QT6-killswitch/resources/` |
| `50.killswitch.rules` | `/usr/local/share/polkit-1/rules.d/` |
| `org.kde.killswitch.policy` | `/usr/local/share/polkit-1/actions/` |

## Usage

```sh
./KDE-QT6-killswitch
```

If not running as root, the application automatically re-launches itself via
`pkexec` and prompts for your password. Root is required because the shutdown
command needs elevated privileges.

Once running:

- Click **Kill Switch ON** to start monitoring.
- Click **Kill Switch OFF** to stop.
- Close the window to minimise to the system tray.
- Right-click the tray icon for **Restore** / **Quit**.

## Polkit configuration

Two files handle privilege management:

### Policy file (`org.kde.killswitch.policy`)

Defines the `org.kde.killswitch.run` action and associates it with the
installed binary path. This is what tells pkexec which action ID to use
instead of the generic `org.freedesktop.policykit.exec`.

### Rules file (`50.killswitch.rules`)

Grants automatic authorisation to users in the `sudo` group so they are not
prompted for a password on every launch. Edit the group name if your system
uses a different admin group (e.g. `wheel`):

```js
polkit.addRule(function(action, subject) {
    if (action.id == "org.kde.killswitch.run" && subject.isInGroup("wheel")) {
        return polkit.Result.YES;
    }
});
```

## Configuration

All configuration is currently in the source code. Key values in
`src/main.cpp`:

| Setting | Default | Location |
|---------|---------|----------|
| DNS servers | `8.8.8.8`, `1.1.1.1`, `208.67.222.222` | `dnsServers` vector |
| Websites | `google.com`, `amazon.com`, `wikipedia.org` | `websites` vector |
| Check interval | 5000-10000 ms (random) | `QRandomGenerator::bounded()` calls |
| Connection timeout | 3000 ms | `checkHost()` timeout parameter |
| Consecutive failures before shutdown | 3 | `kMaxConsecutiveFailures` |

## Project structure

```
.
├── CMakeLists.txt
├── CHANGELOG.md
├── LICENSE
├── README.md
├── polkit/
│   ├── 50.killswitch.rules        # Polkit rules for passwordless launch
│   └── org.kde.killswitch.policy   # Polkit policy defining the action ID
├── resources/
│   └── icon.png                    # System tray icon
└── src/
    └── main.cpp                    # Complete application source
```

## License

[MIT](LICENSE) — Copyright (c) 2025 S.B
