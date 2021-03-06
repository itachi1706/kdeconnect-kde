/**
 * Copyright 2020 Aditya Mehra <aix.m@outlook.com>
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

#include "bigscreenplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>
#include <QDBusConnection>
#include <QLoggingCategory>

#include <core/device.h>
#include <core/daemon.h>

K_PLUGIN_CLASS_WITH_JSON(BigscreenPlugin, "kdeconnect_bigscreen.json")

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_BIGSCREEN, "kdeconnect.plugin.bigscreen")

BigscreenPlugin::BigscreenPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args)
{
}

BigscreenPlugin::~BigscreenPlugin() = default;

bool BigscreenPlugin::receivePacket(const NetworkPacket& np)
{
    QString message = np.get<QString>(QStringLiteral("content"));
    Q_EMIT messageReceived(message);
    return true;
}

QString BigscreenPlugin::dbusPath() const
{
    return QStringLiteral("/modules/kdeconnect/devices/") + device()->id() + QStringLiteral("/bigscreen");
}

#include "bigscreenplugin.moc"
