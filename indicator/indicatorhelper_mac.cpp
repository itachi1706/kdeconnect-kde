/*
 * Copyright 2019 Weixuan XIAO <veyx.shaw@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QStandardPaths>
#include <QThread>
#include <QDebug>

#include <KLocalizedString>

#include <dbushelper.h>

#include "indicatorhelper.h"

#include "serviceregister_mac.h"

IndicatorHelper::IndicatorHelper()
{
    registerServices();

    QIcon kdeconnectIcon = QIcon::fromTheme(QStringLiteral("kdeconnect"));
    QPixmap splashPixmap(kdeconnectIcon.pixmap(256, 256));

    m_splashScreen = new QSplashScreen(splashPixmap);

    m_splashScreen->showMessage(i18n("Launching") + QStringLiteral("\n"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    m_splashScreen->show();
}

IndicatorHelper::~IndicatorHelper()
{
    if (m_splashScreen != nullptr) {
        delete m_splashScreen;
        m_splashScreen = nullptr;
    }
}

void IndicatorHelper::preInit() {}

void IndicatorHelper::postInit()
{
    m_splashScreen->finish(nullptr);
}

void IndicatorHelper::iconPathHook()
{
    const QString iconPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kdeconnect-icons"), QStandardPaths::LocateDirectory);
    if (!iconPath.isNull()) {
        QStringList themeSearchPaths = QIcon::themeSearchPaths();
        themeSearchPaths << iconPath;
        QIcon::setThemeSearchPaths(themeSearchPaths);
    }
}

int IndicatorHelper::daemonHook(QProcess &kdeconnectd)
{
    // Unset launchctl env, avoid block
    DBusHelper::macosUnsetLaunchctlEnv();

    // Start kdeconnectd
    m_splashScreen->showMessage(i18n("Launching daemon") + QStringLiteral("\n"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    if (QFile::exists(QCoreApplication::applicationDirPath() + QStringLiteral("/kdeconnectd"))) {
        kdeconnectd.startDetached(QCoreApplication::applicationDirPath() + QStringLiteral("/kdeconnectd"));
    } else if (QFile::exists(QString::fromLatin1(qgetenv("craftRoot")) + QStringLiteral("/../lib/libexec/kdeconnectd"))) {
        kdeconnectd.startDetached(QString::fromLatin1(qgetenv("craftRoot")) + QStringLiteral("/../lib/libexec/kdeconnectd"));
    } else {
        QMessageBox::critical(nullptr, i18n("KDE Connect"),
                              i18n("Cannot find kdeconnectd"),
                              QMessageBox::Abort,
                              QMessageBox::Abort);
        return -1;
    }

    // Wait for dbus daemon env
    QProcess getLaunchdDBusEnv;
    m_splashScreen->showMessage(i18n("Waiting D-Bus") + QStringLiteral("\n"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    int retry = 0;
    do {
        getLaunchdDBusEnv.setProgram(QStringLiteral("launchctl"));
        getLaunchdDBusEnv.setArguments({
            QStringLiteral("getenv"),
            QStringLiteral(KDECONNECT_SESSION_DBUS_LAUNCHD_ENV)
        });
        getLaunchdDBusEnv.start();
        getLaunchdDBusEnv.waitForFinished();

        QString launchdDBusEnv = QString::fromLocal8Bit(getLaunchdDBusEnv.readAllStandardOutput());

        if (launchdDBusEnv.length() > 0) {
            break;
        } else if (retry >= 10) {
            // Show a warning and exit
            qCritical() << "Fail to get launchctl" << KDECONNECT_SESSION_DBUS_LAUNCHD_ENV << "env";

            QMessageBox::critical(nullptr, i18n("KDE Connect"),
                                  i18n("Cannot connect to DBus\n"
                                  "KDE Connect will quit"),
                                  QMessageBox::Abort,
                                  QMessageBox::Abort);
            return -2;
        } else {
            QThread::sleep(3);  // Retry after 3s
            retry++;
        }
    } while(true);

    m_splashScreen->showMessage(i18n("Loading modules") + QStringLiteral("\n"), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);

    return 0;
}

#ifdef QSYSTRAY
void IndicatorHelper::systrayIconHook(QSystemTrayIcon &systray)
{
    Q_UNUSED(systray);
}
#else
void IndicatorHelper::systrayIconHook(KStatusNotifierItem &systray)
{
    const QString iconPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kdeconnect-icons"), QStandardPaths::LocateDirectory);
    if (!iconPath.isNull()) {
        systray.setIconByName(QStringLiteral("kdeconnectindicatordark"));
    } else {
        // We are in macOS dev env, just continue
        qWarning() << "Fail to find indicator icon, continue anyway";
    }
}
#endif
