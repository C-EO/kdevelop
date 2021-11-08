/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DECLARATION_DATA_H
#define KDEVPLATFORM_DECLARATION_DATA_H

#include "duchainbase.h"

#include "declaration.h"
#include "declarationid.h"
#include "ducontext.h"
#include "topducontext.h"
#include "duchainlock.h"
#include "duchain.h"
#include <language/languageexport.h>
#include "types/indexedtype.h"

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT DeclarationData
    : public DUChainBaseData
{
public:
    DeclarationData();
    DeclarationData(const DeclarationData& rhs) = default;

    IndexedDUContext m_internalContext;
    IndexedType m_type;
    IndexedIdentifier m_identifier;

    ///@todo Eventually move this and all the definition/declaration coupling functionality somewhere else
    //Holds the declaration id for this definition, if this is a definition with separate declaration
    DeclarationId m_declaration;

    //Index in the comment repository
    uint m_comment = 0;

    Declaration::Kind m_kind = Declaration::Instance;

    bool m_isDefinition  : 1;
    bool m_inSymbolTable : 1;
    bool m_isTypeAlias   : 1;
    bool m_anonymousInContext : 1; //Whether the declaration was added into the parent-context anonymously
    bool m_isDeprecated      : 1;
    bool m_alwaysForceDirect : 1;
    bool m_isAutoDeclaration : 1;
    bool m_isExplicitlyDeleted : 1;
    bool m_isExplicitlyTyped : 1;
};
}

#endif
