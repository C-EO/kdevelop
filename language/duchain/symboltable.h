/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <QtCore/QMultiHash>
#include <QtCore/QMultiMap>
#include <languageexport.h>

namespace KDevelop
{

class Declaration;
class DUContext;
class QualifiedIdentifier;

/**
 * A global symbol table, which stores mangled identifiers for quick lookup.
 *
 * \todo profiling, map vs hash etc...
 */
class KDEVPLATFORMLANGUAGE_EXPORT SymbolTable
{
public:
  static SymbolTable* self();

  void dumpStatistics() const;

  // Declarations
  void addDeclaration(Declaration* declaration);
  void removeDeclaration(Declaration* declaration);

  QList<Declaration*> findDeclarations(const QualifiedIdentifier& id) const;
  QList<Declaration*> findDeclarationsBeginningWith(const QualifiedIdentifier& id) const;

  // Named Contexts (classes and namespaces)
  void addContext(DUContext* namedContext);
  void removeContext(DUContext* namedContext);

  QList<DUContext*> findContexts(const QualifiedIdentifier& id) const;

private:
  SymbolTable();
  friend class SymbolTablePrivate;
};

}

#endif // SYMBOLTABLE_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
