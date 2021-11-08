/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "forwarddeclaration.h"

#include <KLocalizedString>

#include "duchain.h"
#include "duchainlock.h"
#include "ducontext.h"
#include "use.h"
#include "duchainregister.h"
#include "types/identifiedtype.h"

using namespace KTextEditor;

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(ForwardDeclaration);

ForwardDeclaration::ForwardDeclaration(const ForwardDeclaration& rhs) : Declaration(*new ForwardDeclarationData(*rhs.
            d_func()))
{
}

ForwardDeclaration::ForwardDeclaration(ForwardDeclarationData& data) : Declaration(data)
{
}

ForwardDeclaration::ForwardDeclaration(const RangeInRevision& range, DUContext* context)
    : Declaration(*new ForwardDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

ForwardDeclaration::~ForwardDeclaration()
{
}

QString ForwardDeclaration::toString() const
{
    if (context())
        return qualifiedIdentifier().toString();
    else
        return i18n("context-free forward-declaration %1", identifier().toString());
}

Declaration* ForwardDeclaration::resolve(const TopDUContext* topContext) const
{
    ENSURE_CAN_READ

    //If we've got a type assigned, that counts as a way of resolution.
    AbstractType::Ptr t = abstractType();
    auto* idType = dynamic_cast<IdentifiedType*>(t.data());
    if (idType) {
        Declaration* decl = idType->declaration(topContext);
        if (decl && !decl->isForwardDeclaration())
            return decl;
        else
            return nullptr;
    }

    if (!topContext)
        topContext = this->topContext();

    QualifiedIdentifier globalIdentifier = qualifiedIdentifier();
    globalIdentifier.setExplicitlyGlobal(true);

    //We've got to use DUContext::DirectQualifiedLookup so C++ works correctly.
    const QList<Declaration*> declarations = topContext->findDeclarations(globalIdentifier,
                                                                          CursorInRevision::invalid(),
                                                                          AbstractType::Ptr(), nullptr,
                                                                          DUContext::DirectQualifiedLookup);

    for (Declaration* decl : declarations) {
        if (!decl->isForwardDeclaration())
            return decl;
    }

    return nullptr;
}

DUContext* ForwardDeclaration::logicalInternalContext(const TopDUContext* topContext) const
{
    ENSURE_CAN_READ
    Declaration* resolved = resolve(topContext);
    if (resolved && resolved != this)
        return resolved->logicalInternalContext(topContext);
    else
        return Declaration::logicalInternalContext(topContext);
}

bool ForwardDeclaration::isForwardDeclaration() const
{
    return true;
}

Declaration* ForwardDeclaration::clonePrivate() const
{
    return new ForwardDeclaration(*this);
}
}
