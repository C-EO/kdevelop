/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "adaptsignatureaction.h"
#include "codegenhelper.h"

#include "../duchain/duchainutils.h"
#include "../util/clangdebug.h"

#include <language/assistant/renameaction.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
// KF
#include <KLocalizedString>

using namespace KDevelop;

AdaptSignatureAction::AdaptSignatureAction(const DeclarationId& definitionId,
                                           const ReferencedTopDUContext& definitionContext,
                                           const Signature& oldSignature,
                                           const Signature& newSignature,
                                           bool editingDefinition,
                                           const QList<RenameAction*>& renameActions)
    : m_otherSideId(definitionId)
    , m_otherSideTopContext(definitionContext)
    , m_oldSignature(oldSignature)
    , m_newSignature(newSignature)
    , m_editingDefinition(editingDefinition)
    , m_renameActions(renameActions)
{
}

AdaptSignatureAction::~AdaptSignatureAction()
{
    qDeleteAll(m_renameActions);
}

QString AdaptSignatureAction::description() const
{
    return m_editingDefinition ? i18n("Update declaration signature") : i18n("Update definition signature");
}

QString AdaptSignatureAction::toolTip() const
{
    DUChainReadLocker lock;
    auto declaration = m_otherSideId.declaration(m_otherSideTopContext.data());
    if (!declaration) {
        return {};
    }
    KLocalizedString msg = m_editingDefinition
                         ? ki18n("Update declaration signature\nfrom: %1\nto: %2")
                         : ki18n("Update definition signature\nfrom: %1\nto: %2");
    msg = msg.subs(CodegenHelper::makeSignatureString(declaration, m_oldSignature, m_editingDefinition));
    msg = msg.subs(CodegenHelper::makeSignatureString(declaration, m_newSignature, !m_editingDefinition));
    return msg.toString();
}

void AdaptSignatureAction::execute()
{
    ENSURE_CHAIN_NOT_LOCKED
    DUChainReadLocker lock;
    IndexedString url = m_otherSideTopContext->url();
    lock.unlock();
    m_otherSideTopContext = DUChain::self()->waitForUpdate(url, TopDUContext::AllDeclarationsContextsAndUses);
    if (!m_otherSideTopContext) {
        clangDebug() << "failed to update" << url.str();
        return;
    }

    lock.lock();

    Declaration* otherSide = m_otherSideId.declaration(m_otherSideTopContext.data());
    if (!otherSide) {
        clangDebug() << "could not find definition";
        return;
    }
    DUContext* functionContext = DUChainUtils::functionContext(otherSide);
    if (!functionContext) {
        clangDebug() << "no function context";
        return;
    }
    if (!functionContext || functionContext->type() != DUContext::Function) {
        clangDebug() << "no correct function context";
        return;
    }

    DocumentChangeSet changes;
    KTextEditor::Range parameterRange = ClangIntegration::DUChainUtils::functionSignatureRange(otherSide);
    QString newText = CodegenHelper::makeSignatureString(otherSide, m_newSignature, !m_editingDefinition);
    if (!m_editingDefinition) {
        // append a newline after the method signature in case the method definition follows
        newText += QLatin1Char('\n');
    }

    DocumentChange changeParameters(functionContext->url(), parameterRange, QString(), newText);
    lock.unlock();
    changeParameters.m_ignoreOldText = true;
    changes.addChange(changeParameters);
    changes.setReplacementPolicy(DocumentChangeSet::WarnOnFailedChange);
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    if (!result) {
        const QString messageText = i18n("Failed to apply changes: %1", result.m_failureReason);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
    emit executed(this);

    for (RenameAction* renAct : m_renameActions) {
        renAct->execute();
    }
}

#include "moc_adaptsignatureaction.cpp"
