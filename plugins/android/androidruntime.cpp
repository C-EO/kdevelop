/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "androidruntime.h"
#include "androidpreferencessettings.h"
#include "debug_android.h"

#include <KProcess>
#include <QProcess>
#include <QUrl>

using namespace KDevelop;

AndroidPreferencesSettings* AndroidRuntime::s_settings = nullptr;

AndroidRuntime::AndroidRuntime()
    : KDevelop::IRuntime()
{
}

AndroidRuntime::~AndroidRuntime()
{
}

void AndroidRuntime::setEnabled(bool /*enable*/)
{
}

static void setEnvironmentVariables(QProcess* process)
{
    auto env = process->processEnvironment();
    env.insert(QStringLiteral("ANDROID_NDK"),      QUrl(AndroidRuntime::s_settings->ndk()).toLocalFile());
    env.insert(QStringLiteral("ANDROID_SDK_ROOT"), QUrl(AndroidRuntime::s_settings->sdk()).toLocalFile());
    process->setProcessEnvironment(env);
}

//integrates with ECM
static QStringList args()
{
    return {
        QLatin1String("-DCMAKE_TOOLCHAIN_FILE=") + QUrl(AndroidRuntime::s_settings->cmakeToolchain()).toLocalFile(),
        QLatin1String("-DANDROID_ABI=") + AndroidRuntime::s_settings->abi(),
        QLatin1String("-DANDROID_NDK=") + QUrl(AndroidRuntime::s_settings->ndk()).toLocalFile(),
        QLatin1String("-DANDROID_TOOLCHAIN=") + AndroidRuntime::s_settings->toolchain(),
        QLatin1String("-DANDROID_API_LEVEL=") + AndroidRuntime::s_settings->api(),
        QLatin1String("-DANDROID_ARCHITECTURE=") + AndroidRuntime::s_settings->arch(),
        QLatin1String("-DANDROID_SDK_BUILD_TOOLS_REVISION=") + AndroidRuntime::s_settings->buildtools()
    };
}

void AndroidRuntime::startProcess(QProcess* process) const
{
    if (process->program().endsWith(QLatin1String("cmake"))) {
        process->setArguments(process->arguments() << args());
        setEnvironmentVariables(process);
    }

    qCDebug(ANDROID) << "starting qprocess" << process->program() << process->arguments();
    process->start();
}

void AndroidRuntime::startProcess(KProcess* process) const
{
    if (process->program().first().endsWith(QLatin1String("cmake"))) {
        process->setProgram(process->program() << args());
        setEnvironmentVariables(process);
    }

    qCDebug(ANDROID) << "starting kprocess" << process->program().join(QLatin1Char(' '));
    process->start();
}

QByteArray AndroidRuntime::getenv(const QByteArray &varname) const
{
    return qgetenv(varname.constData());
}

QString AndroidRuntime::findExecutable(const QString& executableName) const
{
    QStringList rtPaths;

    auto envPaths = getenv(QByteArrayLiteral("PATH")).split(':');
    std::transform(envPaths.begin(), envPaths.end(), std::back_inserter(rtPaths),
                    [this](QByteArray p) {
                        return pathInHost(Path(QString::fromLocal8Bit(p))).toLocalFile();
                    });

    return QStandardPaths::findExecutable(executableName, rtPaths);
}

#include "moc_androidruntime.cpp"
