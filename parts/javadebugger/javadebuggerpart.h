/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DEBUGGERPART_H_
#define _DEBUGGERPART_H_

#include <qguardedptr.h>
#include "kdevpart.h"

class VariableWidget;
class BreakpointWidget;
class FramestackWidget;
class DisassembleWidget;
class Breakpoint;
class DbgController;
class DbgToolBar;

class JavaDebuggerPart : public KDevPart
{
    Q_OBJECT

public:
    JavaDebuggerPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~JavaDebuggerPart();

private slots:
    void slotRun();
    void slotExamineCore();
    void slotAttachProcess();
    void slotStop();
    void slotPause();
    void slotContinue();
    void slotRunToCursor();
    void slotStepOver();
    void slotStepOverInstruction();
    void slotStepIntoInstruction();
    void slotStepInto();
    void slotStepOut();
    void slotMemoryView();

    void slotRefreshBPState(Breakpoint *BP);
    void slotStatus(const QString &msg, int state);
    void slotShowStep(const QString &, int);
    void slotGotoSource(const QString &, int);

private:
    void startDebugger();
    void setupController();
    
    QGuardedPtr<VariableWidget> variableWidget;
    QGuardedPtr<BreakpointWidget> breakpointWidget;
    QGuardedPtr<FramestackWidget> framestackWidget;
    QGuardedPtr<DisassembleWidget> disassembleWidget;
    DbgController *controller;
    //    QGuardedPtr<DbgToolBar> floatingToolBar;
};

#endif
