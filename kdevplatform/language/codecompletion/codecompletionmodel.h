/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODECOMPLETIONMODEL_H
#define KDEVPLATFORM_CODECOMPLETIONMODEL_H

#include <QPair>
#include <QMap>
#include <QPointer>
#include <QExplicitlySharedDataPointer>
#include <QUrl>

#include "../duchain/duchainpointer.h"
#include <language/languageexport.h>
#include "codecompletioncontext.h"
#include "codecompletionitem.h"

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>

class QMutex;

namespace KDevelop {
class DUContext;
class Declaration;
class CodeCompletionWorker;
class CompletionWorkerThread;

class KDEVPLATFORMLANGUAGE_EXPORT CodeCompletionModel
    : public KTextEditor::CodeCompletionModel
    , public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    explicit CodeCompletionModel(QObject* parent);
    ~CodeCompletionModel() override;

    ///This MUST be called after the creation of this completion-model.
    ///If you use the KDevelop::CodeCompletion helper-class, that one cares about it.
    virtual void initialize();

    ///Entry-point for code-completion. This determines ome settings, clears the model, and then calls completionInvokedInternal for further processing.
    void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range,
                           KTextEditor::CodeCompletionModel::InvocationType invocationType) override;

    QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount (const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent (const QModelIndex& index) const override;

    ///Use this to set whether the code-completion widget should wait for this model until it's shown
    ///This makes sense when the model takes some time but not too much time, to make the UI less flickering and
    ///annoying.
    ///The default is false
    ///@todo Remove this option, make it true by default, and make sure in CodeCompletionWorker that the whole thing cannot break
    void setForceWaitForModel(bool wait);

    bool forceWaitForModel();

    ///Convenience-storage for use by the inherited completion model
    void setCompletionContext(const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext);
    QExplicitlySharedDataPointer<CodeCompletionContext> completionContext() const;

    ///Convenience-storage for use by the inherited completion model
    KDevelop::TopDUContextPointer currentTopContext() const;
    void setCurrentTopContext(const KDevelop::TopDUContextPointer& topContext);

    ///Whether the completion should be fully detailed. If false, it should be simplified, so no argument-hints,
    ///no expanding information, no type-information, etc.
    bool fullCompletion() const;

    MatchReaction matchingItem(const QModelIndex& matched) override;

    QString filterString(KTextEditor::View* view, const KTextEditor::Range& range,
                         const KTextEditor::Cursor& position) override;

    void clear();

    ///Returns the tree-element that belongs to the index, or zero
    QExplicitlySharedDataPointer<CompletionTreeElement> itemForIndex(const QModelIndex& index) const;

Q_SIGNALS:
    ///Connection from this completion-model into the background worker thread. You should emit this from within completionInvokedInternal.
    void completionsNeeded(const KDevelop::DUContextPointer& context, const KTextEditor::Cursor& position,
                           KTextEditor::View* view);
    ///Additional signal that allows directly stepping into the worker-thread, bypassing computeCompletions(..) etc.
    ///doSpecialProcessing(data) will be executed in the background thread.
    void doSpecialProcessingInBackground(uint data);

protected Q_SLOTS:
    ///Connection from the background-thread into the model: This is called when the background-thread is ready
    virtual void foundDeclarations(const QList<QExplicitlySharedDataPointer<CompletionTreeElement>>& item,
                                   const QExplicitlySharedDataPointer<CodeCompletionContext>& completionContext);

protected:
    ///Eventually override this, determine the context or whatever, and then emit completionsNeeded(..) to continue processing in the background tread.
    ///The default-implementation does this completely, so if you don't need to do anything special, you can just leave it.
    virtual void completionInvokedInternal(KTextEditor::View* view, const KTextEditor::Range& range,
                                           KTextEditor::CodeCompletionModel::InvocationType invocationType,
                                           const QUrl& url);

    void executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& word,
                               const QModelIndex& index) const override;

    QExplicitlySharedDataPointer<CodeCompletionContext> m_completionContext;
    using DeclarationContextPair = QPair<KDevelop::DeclarationPointer,
        QExplicitlySharedDataPointer<CodeCompletionContext>>;

    QList<QExplicitlySharedDataPointer<CompletionTreeElement>> m_completionItems;

    /// Should create a completion-worker. The worker must have no parent object,
    /// because else its thread-affinity can not be changed.
    virtual CodeCompletionWorker* createCompletionWorker() = 0;
    friend class CompletionWorkerThread;

    CodeCompletionWorker* worker() const;

private:
    bool m_forceWaitForModel;
    bool m_fullCompletion;
    QMutex* m_mutex;
    CompletionWorkerThread* m_thread;
    QString m_filterString;
    KDevelop::TopDUContextPointer m_currentTopContext;
};
}

#endif
