/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configurablecolors.h"

#include "codehighlighting.h"
#include "colorcache.h"

#include <debug.h>

#define ifDebug(x)

namespace KDevelop {
KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::defaultAttribute() const
{
    return m_defaultAttribute;
}

void ConfigurableHighlightingColors::setDefaultAttribute(const KTextEditor::Attribute::Ptr& defaultAttrib)
{
    m_defaultAttribute = defaultAttrib;
}

KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::attribute(int number) const
{
    return m_attributes[number];
}

void ConfigurableHighlightingColors::addAttribute(int number, const KTextEditor::Attribute::Ptr& attribute)
{
    m_attributes[number] = attribute;
}

ConfigurableHighlightingColors::ConfigurableHighlightingColors()
{
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    setDefaultAttribute(a);
}

#define ADD_COLOR(type, color_) \
    { \
        KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute); \
        a->setForeground(QColor(cache->blendGlobalColor(color_)));  \
        addAttribute(CodeHighlighting::type, a);  \
        ifDebug(qCDebug(LANGUAGE) << # type << "color: " << # color_ << "->" << a->foreground().color().name(); ) \
    }

CodeHighlightingColors::CodeHighlightingColors(ColorCache* cache) : ConfigurableHighlightingColors()
{
    // TODO: The set of colors doesn't work very well. Many colors simply too dark (even on the maximum "Global colorization intensity" they hardly distinguishable from grey) and look alike.
    ADD_COLOR(ClassType, 0x005912) //Dark green
    ADD_COLOR(TypeAliasType, 0x35938d)
    ADD_COLOR(EnumType, 0x6c101e) //Dark red
    ADD_COLOR(EnumeratorType, 0x862a38) //Greyish red
    ADD_COLOR(FunctionType, 0x21005A) //Navy blue
    ADD_COLOR(MemberVariableType, 0x443069) //Dark Burple (blue/purple)
    ADD_COLOR(LocalClassMemberType, 0xae7d00) //Light orange
    ADD_COLOR(LocalMemberFunctionType, 0xae7d00)
    ADD_COLOR(InheritedClassMemberType, 0x705000) //Dark orange
    ADD_COLOR(InheritedMemberFunctionType, 0x705000)
    ADD_COLOR(LocalVariableType, 0x0C4D3C)
    ADD_COLOR(FunctionVariableType, 0x300085) //Less dark navy blue
    ADD_COLOR(NamespaceVariableType, 0x9F3C5F) //Rose
    ADD_COLOR(GlobalVariableType, 0x12762B) //Grass green
    ADD_COLOR(NamespaceType, 0x6B2840) //Dark rose
    ADD_COLOR(ErrorVariableType, 0x8b0019) //Pure red
    ADD_COLOR(ForwardDeclarationType, 0x5C5C5C) //Gray
    ADD_COLOR(MacroType, 0xA41239)
    ADD_COLOR(MacroFunctionLikeType, 0x008080)
    ADD_COLOR(HighlightUsesType, 0xffffff)
}
}
