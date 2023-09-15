/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIDEBUGJOBS_H
#define MIDEBUGJOBS_H

#include <outputview/outputjob.h>

#include <QPointer>

class IExecutePlugin;
namespace KDevelop
{
class OutputModel;
class ILaunchConfiguration;
}

namespace KDevMI {

class MIDebuggerPlugin;
class MIDebugSession;

template<class JobBase>
class MIDebugJobBase : public JobBase
{
public:
    explicit MIDebugJobBase(MIDebuggerPlugin* plugin, QObject* parent);
    ~MIDebugJobBase() override;

protected:
    void done();
    bool doKill() override;

    QPointer<MIDebugSession> m_session;
};

class MIDebugJob : public MIDebugJobBase<KDevelop::OutputJob>
{
    Q_OBJECT
public:
    enum {
        // Add a "random" number to KJob::UserDefinedError and hopefully avoid clashes with OutputJob's error codes.
        InvalidExecutable = UserDefinedError + 231,
        ExecutableIsNotExecutable,
        InvalidArguments
    };

    MIDebugJob(MIDebuggerPlugin* p, KDevelop::ILaunchConfiguration* launchcfg, IExecutePlugin* plugin,
             QObject* parent = nullptr);
    void start() override;

private Q_SLOTS:
    void stdoutReceived(const QStringList&);
    void stderrReceived(const QStringList&);

private:
    void finishWithError(int errorCode, const QString& errorText);
    KDevelop::OutputModel* model();

    KDevelop::ILaunchConfiguration* m_launchcfg;
    IExecutePlugin* m_execute;
};

class MIExamineCoreJob : public MIDebugJobBase<KJob>
{
    Q_OBJECT
public:
    explicit MIExamineCoreJob(MIDebuggerPlugin *plugin, QObject *parent = nullptr);

    void start() override;
};

class MIAttachProcessJob : public MIDebugJobBase<KJob>
{
    Q_OBJECT
public:
    MIAttachProcessJob(MIDebuggerPlugin *plugin, int pid, QObject *parent = nullptr);

    void start() override;

private:
    int m_pid;
};

} // end of namespace KDevMI

#endif // MIDEBUGJOBS_H
