// main.cpp

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QTcpSocket>
#include <QRandomGenerator>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QNetworkInterface>
#include <QFileInfo>
#include <QDir>
#include <QCloseEvent>
#include <QIcon>
#include <QDebug>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <unistd.h>  // for geteuid()

#include <vector>

// -----------------------------
// KillSwitchWindow Class
// -----------------------------
class KillSwitchWindow : public QMainWindow {
    Q_OBJECT
public:
    KillSwitchWindow(QWidget *parent = nullptr)
        : QMainWindow(parent), killSwitchEnabled(false), checkInProgress(false), consecutiveFailures(0) {
        setupUI();
        setupTrayIcon();
        setupTimer();
    }

private:
    QWidget *centralWidget;
    QPushButton *toggleButton;
    QCheckBox *fileOperationCheck;
    QCheckBox *upgradeCheck;
    QTimer *checkTimer;
    QSystemTrayIcon *trayIcon;
    bool killSwitchEnabled;
    bool checkInProgress;
    int consecutiveFailures;
    static constexpr int kMaxConsecutiveFailures = 3;

    // Predefined lists for DNS servers and popular websites.
    std::vector<QString> dnsServers = {"8.8.8.8", "1.1.1.1", "208.67.222.222"};
    std::vector<QString> websites   = {"www.google.com", "www.amazon.com", "www.wikipedia.org"};

    // Set up the main UI: toggle button and checklist.
    void setupUI() {
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        // Toggle button that mimics a VPN on/off switch.
        toggleButton = new QPushButton("Kill Switch OFF", this);
        toggleButton->setCheckable(true);
        toggleButton->setStyleSheet("QPushButton { font-size: 18px; padding: 10px; border-radius: 5px; }");
        connect(toggleButton, &QPushButton::clicked, this, &KillSwitchWindow::toggleKillSwitch);
        layout->addWidget(toggleButton);

        // Checklist group to prevent shutdown during operations.
        QGroupBox *checkGroup = new QGroupBox("Prevent Shutdown Conditions", this);
        QVBoxLayout *checkLayout = new QVBoxLayout(checkGroup);
        fileOperationCheck = new QCheckBox("File Operation in Progress", this);
        upgradeCheck = new QCheckBox("System Upgrade/Installation in Progress", this);
        checkLayout->addWidget(fileOperationCheck);
        checkLayout->addWidget(upgradeCheck);
        layout->addWidget(checkGroup);

        setWindowTitle("Linux Kill Switch");
        resize(400, 200);
    }

    // Set up system tray icon with a context menu.
    void setupTrayIcon() {
        trayIcon = new QSystemTrayIcon(this);
        // Attempt to load an icon from the same folder as the executable.
        QString appDir = QCoreApplication::applicationDirPath();
        QString iconPath = appDir + "/icon.png";
        if (QFileInfo::exists(iconPath))
            trayIcon->setIcon(QIcon(iconPath));
        else
            trayIcon->setIcon(QIcon::fromTheme("network-workgroup")); // fallback icon

        trayIcon->setToolTip("Linux Kill Switch");

        QMenu *trayMenu = new QMenu(this);
        QAction *restoreAction = new QAction("Restore", this);
        connect(restoreAction, &QAction::triggered, this, &KillSwitchWindow::showNormal);
        QAction *quitAction = new QAction("Quit", this);
        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
        trayMenu->addAction(restoreAction);
        trayMenu->addAction(quitAction);
        trayIcon->setContextMenu(trayMenu);

        connect(trayIcon, &QSystemTrayIcon::activated, this, &KillSwitchWindow::trayIconActivated);
        trayIcon->show();
    }

    // Set up a QTimer that fires at a random interval between 5 and 10 seconds.
    void setupTimer() {
        checkTimer = new QTimer(this);
        int interval = QRandomGenerator::global()->bounded(5000, 10000);
        checkTimer->setInterval(interval);
        connect(checkTimer, &QTimer::timeout, this, &KillSwitchWindow::performConnectivityCheck);
    }

    // Use a TCP connection to check connectivity for a given host and port.
    bool checkHost(const QString &host, quint16 port, int timeout = 3000) {
        QTcpSocket socket;
        socket.connectToHost(host, port);
        return socket.waitForConnected(timeout);
    }

    // Check whether any local network interface (other than loopback) is up.
    bool localNetworkAvailable() {
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : interfaces) {
            if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
                !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
                return true;
        }
        return false;
    }

private slots:
    // Toggle the kill switch on/off.
    void toggleKillSwitch(bool checked) {
        killSwitchEnabled = checked;
        if (killSwitchEnabled) {
            toggleButton->setText("Kill Switch ON");
            consecutiveFailures = 0;
            checkTimer->start();
        } else {
            toggleButton->setText("Kill Switch OFF");
            checkTimer->stop();
        }
    }

    void performConnectivityCheck() {
        if (!killSwitchEnabled || checkInProgress)
            return;

        if (!localNetworkAvailable()) {
            handleCheckResult(false, false, "Local network unavailable");
            return;
        }

        QString dns = dnsServers.at(QRandomGenerator::global()->bounded(static_cast<quint32>(dnsServers.size())));
        QString site = websites.at(QRandomGenerator::global()->bounded(static_cast<quint32>(websites.size())));

        checkInProgress = true;
        auto *watcher = new QFutureWatcher<QPair<bool, bool>>(this);
        connect(watcher, &QFutureWatcher<QPair<bool, bool>>::finished, this, [this, watcher]() {
            auto result = watcher->result();
            handleCheckResult(result.first, result.second, "Connectivity lost: DNS and website unreachable");
            checkInProgress = false;
            watcher->deleteLater();
        });

        watcher->setFuture(QtConcurrent::run([this, dns, site]() -> QPair<bool, bool> {
            return {checkHost(dns, 53), checkHost(site, 80)};
        }));
    }

    void handleCheckResult(bool dnsOk, bool siteOk, const QString &reason) {
        if (!dnsOk && !siteOk) {
            consecutiveFailures++;
            if (consecutiveFailures >= kMaxConsecutiveFailures) {
                if (!fileOperationCheck->isChecked() && !upgradeCheck->isChecked()) {
                    triggerShutdown(reason);
                } else {
                    qDebug() << "Connectivity lost but pending operations active. Shutdown delayed.";
                }
            } else {
                qDebug() << "Connectivity check failed (" << consecutiveFailures << "/" << kMaxConsecutiveFailures << ")";
            }
        } else {
            consecutiveFailures = 0;
        }
        int interval = QRandomGenerator::global()->bounded(5000, 10000);
        checkTimer->setInterval(interval);
    }

    void triggerShutdown(const QString &reason) {
        qDebug() << "Triggering shutdown due to:" << reason;
        qint64 pid;
        bool ok = QProcess::startDetached("shutdown", QStringList() << "-h" << "now", QString(), &pid);
        if (!ok) {
            qWarning() << "shutdown command failed, trying poweroff";
            ok = QProcess::startDetached("poweroff", QStringList(), QString(), &pid);
            if (!ok)
                qCritical() << "All shutdown methods failed";
        }
    }

    // Handle tray icon activation to show/hide the window.
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (isHidden()) {
                showNormal();
                activateWindow();
            } else {
                hide();
            }
        }
    }

protected:
    // Override close event to hide window instead of exiting.
    void closeEvent(QCloseEvent *event) override {
        if (trayIcon->isVisible()) {
            hide();
            event->ignore();
        } else {
            QMainWindow::closeEvent(event);
        }
    }
};

#include "main.moc"

// -----------------------------
// Main Function with Admin Elevation
// -----------------------------
int main(int argc, char *argv[]) {
    // Check if the process is running as root.
    if(geteuid() != 0) {
        QString program = QString::fromLocal8Bit(argv[0]);
        QStringList args;
        for (int i = 1; i < argc; ++i)
            args << QString::fromLocal8Bit(argv[i]);

        qint64 pid;
        bool ok = QProcess::startDetached("pkexec", QStringList() << program << args, QString(), &pid);
        if (!ok) {
            qCritical() << "Failed to relaunch with pkexec — is polkit installed?";
            return 1;
        }
        return 0;
    }

    // Running as root, launch the application.
    QApplication app(argc, argv);

    // Set a light-themed style; you can extend this to support dark mode.
    app.setStyleSheet("QMainWindow { background-color: #f0f0f0; }"
                      "QPushButton { background-color: #009688; color: white; border-radius: 5px; padding: 10px; }"
                      "QCheckBox { font-size: 14px; }");

    KillSwitchWindow window;
    window.show();

    return app.exec();
}
