/*
    SPDX-FileCopyrightText: 2012 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_JSONDUCONTEXTTESTS_H
#define KDEVPLATFORM_JSONDUCONTEXTTESTS_H

#include "language/duchain/ducontext.h"
#include "jsontesthelpers.h"

/**
 * JSON Object Specification:
 * FindDeclObject: Mapping of (string) search ids to DeclTestObjects
 * IndexDeclObject: Mapping of (string) declaration position indexes to DeclTestObjects
 * IndexCtxtObject: Mapping of (string) context position indexes to CtxtTestObjects
 *
 * Quick Reference:
 *   findDeclarations : FindDeclObject
 *   declarations : IndexDeclObject
 *   childCount : int
 *   localDeclarationCount : int
 *   type : string
 *   null : bool
 *   owner : DeclTestObject
 *   importedParents : IndexCtxtObject
 *   range : string
 */

namespace KDevelop {
namespace DUContextTests {
using namespace JsonTestHelpers;

///JSON type: FindDeclObject
///@returns whether each declaration can be found and passes its tests
ContextTest(findDeclarations)
{
    VERIFY_TYPE(QVariantMap);
    const QString INVALID_ERROR = QStringLiteral("Attempted to test invalid context.");
    const QString NOT_FOUND_ERROR = QStringLiteral("Could not find declaration \"%1\".");
    const QString DECL_ERROR = QStringLiteral("Declaration found with \"%1\" did not pass tests.");
    if (!ctxt)
        return INVALID_ERROR;
    QVariantMap findDecls = value.toMap();
    for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it) {
        QualifiedIdentifier searchId(it.key());
        QList<Declaration*> ret = ctxt->findDeclarations(searchId, CursorInRevision::invalid());
        if (!ret.size())
            return NOT_FOUND_ERROR.arg(it.key());

        if (!runTests(it.value().toMap(), ret.first()))
            return DECL_ERROR.arg(it.key());
    }

    return SUCCESS();
}
///JSON type: IndexDeclObject
///@returns whether a declaration exists at each index and each declaration passes its tests
ContextTest(declarations)
{
    VERIFY_TYPE(QVariantMap);
    const QString INVALID_ERROR = QStringLiteral("Attempted to test invalid context.");
    const QString NOT_FOUND_ERROR = QStringLiteral("No declaration at index \"%1\".");
    const QString DECL_ERROR = QStringLiteral("Declaration at index \"%1\" did not pass tests.");
    if (!ctxt)
        return INVALID_ERROR;
    QVariantMap findDecls = value.toMap();
    for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it) {
        int index = it.key().toInt();
        QVector<Declaration*> decls = ctxt->localDeclarations(nullptr);
        if (decls.size() <= index)
            return NOT_FOUND_ERROR;

        if (!runTests(it.value().toMap(), decls.at(index)))
            return DECL_ERROR.arg(it.key());
    }

    return SUCCESS();
}
///JSON type: int
///@returns whether the number of child contexts matches the given value
ContextTest(childCount)
{
    return compareValues(ctxt->childContexts().size(), value, QStringLiteral("Context's child count"));
}
///JSON type: int
///@returns whether the number of local declarations matches the given value
ContextTest(localDeclarationCount)
{
    return compareValues(ctxt->localDeclarations().size(), value, QStringLiteral("Context's local declaration count"));
}
///JSON type: string
///@returns whether the context's type matches the given value
ContextTest(type)
{
    QString contextTypeString;
    switch (ctxt->type()) {
    case DUContext::Class: contextTypeString = QStringLiteral("Class"); break;
    case DUContext::Enum: contextTypeString = QStringLiteral("Enum"); break;
    case DUContext::Namespace: contextTypeString = QStringLiteral("Namespace"); break;
    case DUContext::Function: contextTypeString = QStringLiteral("Function"); break;
    case DUContext::Template: contextTypeString = QStringLiteral("Template"); break;
    case DUContext::Global: contextTypeString = QStringLiteral("Global"); break;
    case DUContext::Helper: contextTypeString = QStringLiteral("Helper"); break;
    case DUContext::Other: contextTypeString = QStringLiteral("Other"); break;
    }
    return compareValues(contextTypeString, value, QStringLiteral("Context's type"));
}
///JSON type: bool
///@returns whether the context's nullity matches the given value
ContextTest(null)
{
    return compareValues(ctxt == nullptr, value, QStringLiteral("Context's nullity"));
}

//JSON type: DeclTestObject
///@returns the context's owner
ContextTest(owner)
{
    return testObject(ctxt->owner(), value, QStringLiteral("Context's owner"));
}

///JSON type: IndexCtxtObject
///@returns whether a context exists at each index and each context passes its tests
ContextTest(importedParents)
{
    VERIFY_TYPE(QVariantMap);
    const QString INVALID_ERROR = QStringLiteral("Attempted to test invalid context.");
    const QString NOT_FOUND_ERROR = QStringLiteral("No imported context at index \"%1\".");
    const QString CONTEXT_ERROR = QStringLiteral("Context at index \"%1\" did not pass tests.");
    if (!ctxt)
        return INVALID_ERROR;
    QVariantMap findDecls = value.toMap();
    for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it) {
        int index = it.key().toInt();
        QVector<DUContext::Import> imports = ctxt->importedParentContexts();
        if (imports.size() <= index)
            return NOT_FOUND_ERROR;

        if (!runTests(it.value().toMap(), imports.at(index).context(ctxt->topContext())))
            return CONTEXT_ERROR.arg(it.key());
    }

    return SUCCESS();
}

///JSON type: string
///@returns stringified context range
ContextTest(range)
{
    if (!ctxt) {
        return QStringLiteral("Invalid Context");
    }
    return compareValues(rangeStr(ctxt->range()), value, QStringLiteral("Contexts's range"));
}
}
}

#endif //KDEVPLATFORM_JSONDUCONTEXTTESTS_H
