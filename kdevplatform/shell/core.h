/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Kris Wong <kris.p.wong@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CORE_H
#define KDEVPLATFORM_CORE_H

#include "shellexport.h"
#include <interfaces/icore.h>

namespace KDevelop {

class UiController;
class PluginController;
class ProjectController;
class LanguageController;
class PartController;
class DocumentController;
class RunController;
class SessionController;
class CorePrivate;
class SourceFormatterController;
class ProgressManager;
class SelectionController;
class DocumentationController;
class DebugController;
class WorkingSetController;
class RuntimeController;
class TestController;

class KDEVPLATFORMSHELL_EXPORT Core: public ICore
{
    Q_OBJECT

public:
    enum Setup { Default=0, NoUi=1 };

    static QString version();

    /** Initialize the core of the kdevplatform application
      * returns false if the initialization fails, which may happen
      * if the same session is already active in another instance
      *
      * @param mode the mode in which to run
      * @param session the name or uuid of the session to be loaded
      *
      */
    static bool initialize(Setup mode=Default, const QString& session = {});

    /**
     * \brief Provide access an instance of Core
     */
    static Core *self();

    ~Core() override;

    IUiController *uiController() override;
    IPluginController *pluginController() override;
    IProjectController *projectController() override;
    ILanguageController *languageController() override;
    IPartController *partController() override;
    IDocumentController *documentController() override;
    IRunController *runController() override;
    ISourceFormatterController* sourceFormatterController() override;
    ISelectionController* selectionController() override;
    IDocumentationController* documentationController() override;
    IDebugController* debugController() override;
    ITestController* testController() override;
    IRuntimeController* runtimeController() override;
    ISession *activeSession() override;
    ISessionLock::Ptr activeSessionLock() override;
    QString sessionTemporaryDirectoryPath() const override;

    /// The following methods may only be used within the shell.

    UiController *uiControllerInternal();
    PluginController *pluginControllerInternal();
    ProjectController *projectControllerInternal();
    LanguageController *languageControllerInternal();
    PartController *partControllerInternal();
    DocumentController *documentControllerInternal();
    RunController *runControllerInternal();
    DocumentationController *documentationControllerInternal();
    DebugController *debugControllerInternal();
    WorkingSetController* workingSetControllerInternal();
    SourceFormatterController* sourceFormatterControllerInternal();
    TestController* testControllerInternal();
    RuntimeController* runtimeControllerInternal();

    /// @internal
    SessionController *sessionController();
    /// @internal
    ProgressManager *progressController();

    void cleanup();

    bool shuttingDown() const override;

    Core::Setup setupFlags() const;

public Q_SLOTS:
    void shutdown();

protected:
    friend class CorePrivate;
    explicit Core( KDevelop::CorePrivate* dd, QObject* parent = nullptr );
    KDevelop::CorePrivate *d;
    static Core *m_self;
private:
    explicit Core(QObject *parent = nullptr);
};

}

#endif

