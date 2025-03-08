# KDE KillSwitch Daemon

**A robust and secure connectivity kill switch daemon for Linux KDE environments.**

A lightweight, Qt6-based daemon that continuously monitors your system's connectivity. If connectivity to critical endpoints is lost—and no important operations (like file transfers or system upgrades) are in progress—the daemon triggers an immediate shutdown. Designed to integrate seamlessly into KDE environments, it offers a modern GUI, system tray functionality, and automatic administrative elevation via pkexec.

---


## Overview

The **KDE KillSwitch Daemon** is engineered to enhance system security by monitoring internet connectivity in real time. The daemon performs connectivity tests at random intervals (between 5 and 10 seconds) against a set of predefined DNS servers and popular websites. If both tests fail and no critical operations are in progress, it initiates an immediate shutdown. 

The application features:
- A modern, KDE-styled UI with both light and dark theme options.
- A system tray integration that allows the daemon to run seamlessly in the background.
- Automatic elevation to root privileges using `pkexec`, with the potential for persistent administration via Polkit rules.

---

## Features

- **Real-Time Connectivity Monitoring:**  
  Randomly verifies connectivity to multiple critical endpoints (DNS servers and websites) every 5–10 seconds.

- **Daemon Mode & System Tray Support:**  
  Runs in the background with a system tray icon for quick access and control.

- **Administrative Elevation:**  
  Automatically re-launches with root privileges via `pkexec` if not already elevated.  
  Optionally, use Polkit rules to avoid repeated password prompts.

- **Prevent Shutdown Conditions:**  
  Checklist options to prevent shutdown during file operations or system upgrades.

- **KDE Integration:**  
  Designed to match KDE aesthetics with a modern, intuitive GUI and native system tray behavior.

- **Customizable:**  
  Easily configure endpoints, timer intervals, and UI themes by modifying the source.

---

## Installation

### Building with QMake

1. **Clone the Repository:**

   ```
   git clone https://github.com/s-b-repo/KDE-QT6-killswitch.git
   cd kde-killswitch-daemon
   ```

2. **Generate Project File and Build:**

   ```
   qmake -project
   qmake
   make
   ```

3. **Run the Application:**

   ```
   ./kde-killswitch-daemon
   ```

### Building with CMake

1. **Clone the Repository:**

   ```
   git clone https://github.com/s-b-repo/KDE-QT6-killswitch.git
   cd kde-killswitch-daemon
   ```

2. **Create a Build Directory:**

   ```
   mkdir build && cd build
   ```

3. **Run CMake and Build:**

   ```
   cmake ..
   make
   ```

4. **Run the Application:**

   ```
   ./kde-killswitch-daemon
   ```

### Dependencies

- **Qt6:** Used for the GUI, networking, and timer functionalities.
- **KDE Frameworks (Optional):** For deeper KDE integration.
- **Polkit & pkexec:** For administrative elevation.
- **Linux Standard Utilities:** Ensure availability of the `shutdown` command.

---

## Usage

- **Launching the Application:**  
  On first launch, if not run as root, the application will automatically re-launch using `pkexec` to prompt for administrative credentials.

- **Activating the Kill Switch:**  
  Toggle the on-screen switch to enable or disable the kill switch functionality.

- **Prevent Shutdown During Critical Operations:**  
  Use the provided checkboxes to delay a shutdown during file transfers or system upgrades.

- **System Tray Integration:**  
  The application minimizes to the system tray. A left-click on the tray icon will restore or hide the window, and a context menu provides options to restore or quit.

---

## Administration and Polkit Integration

To ensure the application runs with elevated privileges without prompting for a password on every launch, you can configure Polkit:

1. **Create a Polkit Rule:**

   Create a file at `/etc/polkit-1/rules.d/50.kde-killswitch.rules` with the following content:

   ```js
   polkit.addRule(function(action, subject) {
       if (action.id == "org.kde.killswitch.run" && subject.isInGroup("yourGroupName")) {
           return polkit.Result.YES;
       }
   });
   ```

2. **Create a Policy File:**

   Place a policy file (e.g., `org.kde.killswitch.policy`) in `/usr/share/polkit-1/actions/` to define the action `org.kde.killswitch.run`.

3. **Security Considerations:**  
   Persistent elevation should be managed with caution. Ensure that only trusted users or groups are granted this privilege.

---

## Configuration

- **Endpoints:**  
  Modify the list of DNS servers and websites in the source code to suit your network environment:

  ```cpp
  std::vector<QString> dnsServers = {"8.8.8.8", "1.1.1.1", "208.67.222.222"};
  std::vector<QString> websites = {"www.google.com", "www.amazon.com", "www.wikipedia.org"};
  ```

- **Timer Settings:**  
  The timer interval is randomized between 5 and 10 seconds. Adjust these values in the source code if needed.

- **UI Themes:**  
  The default stylesheet is set to a light theme. Enhance the application by adding a dark theme option through additional stylesheet settings.

---

## Architecture and Code Overview

- **Main Components:**
  - **KillSwitchWindow:**  
    The central UI component that handles connectivity checks, system tray integration, and shutdown triggers.
  
  - **Admin Elevation:**  
    The main function checks for root privileges using `geteuid()` and relaunches with `pkexec` if needed.
  
  - **Connectivity Checker:**  
    Uses TCP socket connections to verify access to key endpoints and triggers a shutdown if both checks fail.
  
  - **System Tray Integration:**  
    Allows the application to run in the background while providing a convenient interface via a tray icon.

- **Code Structure:**
  - **main.cpp:** Contains the complete implementation, including UI setup, connectivity checks, admin elevation, and system tray handling.
  - **UI Elements:** Utilizes Qt Widgets for a modern, KDE-friendly interface.
  - **Network Operations:** Uses Qt's networking modules for reliable connectivity testing.
  - **Shutdown Mechanism:** Leverages QProcess to execute the `shutdown -h now` command, requiring appropriate system privileges.

---

## Contributing

Contributions are welcome! .

---

## License

This project is licensed under the [MIT License](LICENSE). See the LICENSE file for full details.

---

## Acknowledgments

- **Qt Community:** For providing a robust framework and extensive documentation.
- **KDE Developers:** For the inspiration and continuous improvements in the KDE ecosystem.
- **Freedesktop.org:** For tools such as Polkit that enable secure privilege management.

---

This README aims to provide a comprehensive guide to the KDE KillSwitch Daemon. Feedback and contributions are highly appreciated. Let’s work together to enhance system security in KDE environments!

---

Feel free to adjust any sections to better suit your project’s specifics or the needs of the KDE community. Enjoy coding!
