/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "projectitemquickopen.h"

#include <QIcon>

#include <KLocalizedString>

#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

#include <language/interfaces/iquickopen.h>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/functiondefinition.h>

using namespace KDevelop;

DUChainItemData::DUChainItemData(const DUChainItem& file, bool openDefinition)
    : m_item(file)
    , m_openDefinition(openDefinition)
{
}

QString DUChainItemData::text() const
{
    DUChainReadLocker lock;
    Declaration* decl = m_item.m_item.data();
    if (!decl) {
        return i18n("Not available any more: %1", m_item.m_text);
    }

    if (auto* def = dynamic_cast<FunctionDefinition*>(decl)) {
        if (def->declaration()) {
            decl = def->declaration();
        }
    }

    QString text = decl->qualifiedIdentifier().toString();

    if (!decl->abstractType()) {
        //With simplified representation, still mark functions as such by adding parens
        if (dynamic_cast<AbstractFunctionDeclaration*>(decl)) {
            text += QLatin1String("(...)");
        }
    } else if (TypePtr<FunctionType> function = decl->type<FunctionType>()) {
        text += function->partToString(FunctionType::SignatureArguments);
    }

    return text;
}

QList<QVariant> DUChainItemData::highlighting() const
{
    DUChainReadLocker lock;

    Declaration* decl = m_item.m_item.data();
    if (!decl) {
        return QList<QVariant>();
    }

    if (auto* def = dynamic_cast<FunctionDefinition*>(decl)) {
        if (def->declaration()) {
            decl = def->declaration();
        }
    }

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    QTextCharFormat normalFormat;

    int prefixLength = 0;

    QString signature;
    TypePtr<FunctionType> function = decl->type<FunctionType>();
    if (function) {
        signature = function->partToString(FunctionType::SignatureArguments);
    }

    //Only highlight the last part of the qualified identifier, so the scope doesn't distract too much
    QualifiedIdentifier id = decl->qualifiedIdentifier();
    QString fullId = id.toString();
    QString lastId;
    if (!id.isEmpty()) {
        lastId = id.last().toString();
    }

    prefixLength += fullId.length() - lastId.length();

    QList<QVariant> ret{
        0,
        prefixLength,
        QVariant(normalFormat),
        prefixLength,
        lastId.length(),
        QVariant(boldFormat),
    };
    if (!signature.isEmpty()) {
        ret << prefixLength + lastId.length();
        ret << signature.length();
        ret << QVariant(normalFormat);
    }

    return ret;
}

QString DUChainItemData::htmlDescription() const
{
    if (m_item.m_noHtmlDestription) {
        return QString();
    }

    DUChainReadLocker lock;
    Declaration* decl = m_item.m_item.data();
    if (!decl) {
        return i18n("Not available any more");
    }

    TypePtr<FunctionType> function = decl->type<FunctionType>();

    QString text;

    if (function && function->returnType()) {
        text = i18nc("%1: function signature", "Return: %1",
                     function->partToString(FunctionType::SignatureReturn)) + QLatin1Char(' ');
    }

    text += i18nc("%1: file path", "File: %1", ICore::self()->projectController()->prettyFileName(decl->url().toUrl()));

    QString ret = QLatin1String("<small><small>") + text + QLatin1String("</small></small>");

    return ret;
}

bool DUChainItemData::execute(QString& /*filterText*/)
{
    DUChainReadLocker lock;
    Declaration* decl = m_item.m_item.data();
    if (!decl) {
        return false;
    }

    if (m_openDefinition && FunctionDefinition::definition(decl)) {
        decl = FunctionDefinition::definition(decl);
    }

    QUrl url = decl->url().toUrl();
    KTextEditor::Cursor cursor = decl->rangeInCurrentRevision().start();

    DUContext* internal = decl->internalContext();

    if (internal && (internal->type() == DUContext::Other || internal->type() == DUContext::Class)) {
        //Move into the body
        if (internal->range().end.line > internal->range().start.line) {
            cursor = KTextEditor::Cursor(internal->range().start.line + 1, 0); //Move into the body
        }
    }

    lock.unlock();
    ICore::self()->documentController()->openDocument(url, cursor);
    return true;
}

bool DUChainItemData::isExpandable() const
{
    return true;
}

QWidget* DUChainItemData::expandingWidget() const
{
    DUChainReadLocker lock;

    auto* decl = dynamic_cast<KDevelop::Declaration*>(m_item.m_item.data());
    if (!decl || !decl->context()) {
        return nullptr;
    }

    return decl->context()->createNavigationWidget(decl, decl->topContext(),
                                                   AbstractNavigationWidget::EmbeddableWidget);
}

QIcon DUChainItemData::icon() const
{
    return QIcon();
}

Path DUChainItemData::projectPath() const
{
    return m_item.m_projectPath;
}

DUChainItemDataProvider::DUChainItemDataProvider(IQuickOpen* quickopen, bool openDefinitions)
    : m_quickopen(quickopen)
    , m_openDefinitions(openDefinitions)
{
    reset();
}

void DUChainItemDataProvider::setFilterText(const QString& text)
{
    Base::setFilter(text);
}

uint DUChainItemDataProvider::itemCount() const
{
    return Base::filteredItems().count();
}

uint DUChainItemDataProvider::unfilteredItemCount() const
{
    return Base::items().count();
}

QuickOpenDataPointer DUChainItemDataProvider::data(uint row) const
{
    return KDevelop::QuickOpenDataPointer(createData(Base::filteredItems()[row]));
}

DUChainItemData* DUChainItemDataProvider::createData(const DUChainItem& item) const
{
    return new DUChainItemData(item, m_openDefinitions);
}

QString DUChainItemDataProvider::itemText(const DUChainItem& data) const
{
    return data.m_text;
}

void DUChainItemDataProvider::reset()
{
}
