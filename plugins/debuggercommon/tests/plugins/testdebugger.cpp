/*
    SPDX-FileCopyrightText: 2020 Christoph Roick <chrisito@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**
 * Required implementations to inherit from MIDebuggerPlugin to test the DBus interface
 */

#include <KPluginFactory>

#include "../mibreakpointcontroller.h"
#include "../midebugger.h"
#include "../midebuggerplugin.h"
#include "../midebugsession.h"

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <tests/testframestackmodel.h>
#include <tests/testvariablecontroller.h>

#include <QObject>
#include <QTimer>

class TestDebugger : public KDevMI::MIDebugger
{
    Q_OBJECT
public:
    TestDebugger() : KDevMI::MIDebugger() {}
    bool start(KConfigGroup&, const QStringList&) override { return true; }
};

class TestDebugSession : public KDevMI::MIDebugSession
{
    Q_OBJECT
public:
    explicit TestDebugSession(KDevMI::MIDebuggerPlugin* plugin = nullptr)
        : KDevMI::MIDebugSession(plugin)
        , m_breakpointController{new KDevMI::MIBreakpointController(this)}
        , m_variableController{new KDevelop::TestVariableController(this)}
        , m_frameStackModel{new KDevelop::TestFrameStackModel(this)}
    {
    }

protected:
    KDevMI::MIDebugger *createDebugger() const override
    {
        return new TestDebugger;
    }
    void initializeDebugger() override
    {
        QTimer::singleShot(500, this, &TestDebugSession::finished);
    }
    void configInferior(KDevelop::ILaunchConfiguration *, IExecutePlugin *, const QString &) override {}
    bool execInferior(KDevelop::ILaunchConfiguration *, IExecutePlugin *, const QString &) override { return false; }
    bool loadCoreFile(KDevelop::ILaunchConfiguration *, const QString &, const QString &) override { return false; }

    KDevMI::MIBreakpointController* breakpointController() const override
    {
        return m_breakpointController;
    }
    KDevelop::IVariableController* variableController() const override
    {
        return m_variableController;
    }
    KDevelop::IFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    KDevMI::MIBreakpointController* const m_breakpointController;
    KDevelop::IVariableController* const m_variableController;
    KDevelop::IFrameStackModel* const m_frameStackModel;
};

class TestDebuggerPlugin : public KDevMI::MIDebuggerPlugin
{
    Q_OBJECT
public:
    TestDebuggerPlugin(QObject *parent, const QVariantList&) : MIDebuggerPlugin("testdebugger", "Test", parent) {}
    ~TestDebuggerPlugin() {}

    KDevMI::MIDebugSession *createSession() override
    {
        auto* session = new TestDebugSession(this);
        KDevelop::ICore::self()->debugController()->addSession(session);
        connect(session, &TestDebugSession::showMessage, this, &TestDebuggerPlugin::showStatusMessage);
        return session;
    }
    void setupToolViews() override {}
    void unloadToolViews() override {}
};

K_PLUGIN_FACTORY_WITH_JSON(TestDebuggerFactory, "testdebugger.json", registerPlugin<TestDebuggerPlugin>(); )

#include "testdebugger.moc"
