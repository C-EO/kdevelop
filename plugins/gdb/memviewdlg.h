/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MEMVIEW_H_
#define MEMVIEW_H_

#include "dbgglobal.h"
#include "mi/mi.h"

#include <QWidget>

namespace Okteta {
class ByteArrayModel;
}
namespace Okteta {
class ByteArrayColumnView;
}

namespace KDevelop {
class IDebugSession;
}

class QToolBox;


namespace KDevMI
{
namespace GDB
{
    class CppDebuggerPlugin;
    class MemoryView;
    class GDBController;
    class MemoryRangeSelector;

    class MemoryViewerWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit MemoryViewerWidget(CppDebuggerPlugin* plugin, QWidget* parent = nullptr);

    public Q_SLOTS:
        /** Adds a new memory view. */
        void slotAddMemoryView();

    Q_SIGNALS:
        void requestRaise();


    private Q_SLOTS:
        void slotChildCaptionChanged(const QString& caption);

    private: // Data
        QToolBox* m_toolBox;
    };

    class MemoryView : public QWidget
    {
        Q_OBJECT
    public:
        explicit MemoryView(QWidget* parent);

        void debuggerStateChanged(DBGStateFlags state);

    Q_SIGNALS:
        void captionChanged(const QString& caption);

    private: // Callbacks
        void sizeComputed(const QString& value);

        void memoryRead(const MI::ResultRecord& r);

        // Returns true is we successfully created the memoryView, and
        // can work.
        bool isOk() const;

    private Q_SLOTS:
        void memoryEdited(int start, int end);
        /** Informs the view about changes in debugger state.
         *  Allows view to disable itself when debugger is not running. */
        void slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

        /** Invoked when user has changed memory range.
            Gets memory for the new range. */
        void slotChangeMemoryRange();
        void slotHideRangeDialog();
        void slotEnableOrDisable();

    private: // QWidget overrides
        void contextMenuEvent(QContextMenuEvent* e) override;

        void initWidget();

        MemoryRangeSelector* m_rangeSelector;
        Okteta::ByteArrayModel *m_memViewModel;
        Okteta::ByteArrayColumnView *m_memViewView;

        quintptr m_memStart;
        QString m_memStartStr, m_memAmountStr;
        QByteArray m_memData;
        int m_debuggerState;

    private Q_SLOTS:
        void currentSessionChanged(KDevelop::IDebugSession* session);
    };

} // end of namespace GDB
} // end of namespace KDevMI

#endif
