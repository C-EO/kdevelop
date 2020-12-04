/* This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KDEV_TESTMICOMMAND_H
#define KDEV_TESTMICOMMAND_H

#include <QObject>

class TestMICommand : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testUserCommand();
    void testMICommandHandler_data();
    void testMICommandHandler();
    void testQObjectCommandHandler();
};

#endif
