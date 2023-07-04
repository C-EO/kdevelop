/*
    SPDX-FileCopyrightText: 2013 David E. Narvaez <david@piensalibre.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "stashpatchsource.h"
#include "gitplugin.h"

#include "vcs/dvcs/dvcsjob.h"
#include "interfaces/icore.h"
#include "interfaces/iruncontroller.h"

#include <QTemporaryFile>
#include <QTextStream>

using namespace KDevelop;

StashPatchSource::StashPatchSource(const QString& stashName, GitPlugin* plugin, const QDir & baseDir)
 : m_stashName(stashName), m_plugin(plugin), m_baseDir(baseDir)
{
    QTemporaryFile tempFile;

    tempFile.setAutoRemove(false);
    tempFile.open();
    m_patchFile = QUrl::fromLocalFile(tempFile.fileName());

    auto job = qobject_cast<KDevelop::DVcsJob*>(m_plugin->gitStash(m_baseDir, QStringList{QStringLiteral("show"), QStringLiteral("-u"), m_stashName}, KDevelop::OutputJob::Silent));

    connect(job, &DVcsJob::resultsReady, this, &StashPatchSource::updatePatchFile);
    KDevelop::ICore::self()->runController()->registerJob(job);
}

StashPatchSource::~StashPatchSource()
{
    QFile::remove(m_patchFile.toLocalFile());
}

QUrl StashPatchSource::baseDir() const
{
    return QUrl::fromLocalFile(m_baseDir.absolutePath());
}

QUrl StashPatchSource::file() const
{
    return m_patchFile;
}

void StashPatchSource::update()
{
}

bool StashPatchSource::isAlreadyApplied() const
{
    return false;
}

QString StashPatchSource::name() const
{
    return m_stashName;
}

void StashPatchSource::updatePatchFile(KDevelop::VcsJob* job)
{
    auto* dvcsJob = qobject_cast<KDevelop::DVcsJob*>(job);
    QFile f(m_patchFile.toLocalFile());
    QTextStream txtStream(&f);

    f.open(QIODevice::WriteOnly);
    txtStream << dvcsJob->rawOutput();
    f.close();

    emit patchChanged();
}

#include "moc_stashpatchsource.cpp"
