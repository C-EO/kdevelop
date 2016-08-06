/*
 * Unit tests for LLDB debugger plugin
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LLDBFORMATTERSTEST_H
#define LLDBFORMATTERSTEST_H

#include <QObject>

class LldbFormattersTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testQString();
    void testQByteArray();
    void testQListContainer_data();
    void testQListContainer();
    void testQMapInt();
    void testQMapString();
    void testQMapStringBool();
    void testQDate();
    void testQTime();
    void testQDateTime();
    void testQUrl();
    void testQHashInt();
    void testQHashString();
    void testQSetInt();
    void testQSetString();
    void testQChar();
    void testQListPOD();
    void testQUuid();
    void testKTextEditorTypes();
    void testKDevelopTypes();
};

#endif
