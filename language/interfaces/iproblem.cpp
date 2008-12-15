/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "iproblem.h"
#include <duchain/duchainregister.h>

namespace KDevelop {
REGISTER_DUCHAIN_ITEM(Problem);
}

using namespace KDevelop;

Problem::Problem()
    : DUChainBase(*new ProblemData)
{
    d_func_dynamic()->setClassId(this);
}

KDevelop::Problem::Problem(KDevelop::ProblemData& data) : DUChainBase(data) {
}

KDevelop::IndexedString KDevelop::Problem::url() const
{
    return d_func()->url;
}

DocumentRange Problem::finalLocation() const
{
    return DocumentRange(d_func()->url.str(), range().textRange());
}

void Problem::setFinalLocation(const DocumentRange & location)
{
    setRange(SimpleRange(location));
    d_func_dynamic()->url = IndexedString(location.document().str());
}

QStack< DocumentCursor > Problem::locationStack() const
{
    return QStack< DocumentCursor >();
//     return d_func()->locationStack;
}

void Problem::addLocation(const DocumentCursor & cursor)
{
//     d_func()->locationStack.push(DocumentCursor(cursor));
}

void Problem::clearLocationStack()
{
//     d_func()->locationStack.clear();
}

void Problem::setLocationStack(const QStack< DocumentCursor > & locationStack)
{
//     d_func()->locationStack = locationStack;
}

QString Problem::description() const
{
    return d_func()->description.str();
}

void Problem::setDescription(const QString & description)
{
    d_func_dynamic()->description = ReferenceCountedIndexedString(description);
}

QString Problem::explanation() const
{
    return d_func()->explanation.str();
}

void Problem::setExplanation(const QString & explanation)
{
    d_func_dynamic()->explanation = ReferenceCountedIndexedString(explanation);
}

ProblemData::Source Problem::source() const
{
    return d_func()->source;
}

void Problem::setSource(ProblemData::Source source)
{
    d_func_dynamic()->source = source;
}

/*
  public QSharedData
  QSharedDataPointer<Data> d;
*/
