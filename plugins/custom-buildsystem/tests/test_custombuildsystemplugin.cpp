/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "test_custombuildsystemplugin.h"

#include <QTest>
#include <QDebug>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/kdevsignalspy.h>
#include <shell/sessioncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <serialization/indexedstring.h>

#include "testconfig.h"

using KDevelop::Core;
using KDevelop::ICore;
using KDevelop::IProject;
using KDevelop::TestCore;
using KDevelop::AutoTestShell;
using KDevelop::KDevSignalSpy;
using KDevelop::Path;

QTEST_MAIN(TestCustomBuildSystemPlugin)

void TestCustomBuildSystemPlugin::cleanupTestCase()
{
    TestCore::shutdown();
}
void TestCustomBuildSystemPlugin::initTestCase()
{
    AutoTestShell::init({"KDevCustomBuildSystem", "KDevStandardOutputView"});
    TestCore::initialize();
}

void TestCustomBuildSystemPlugin::loadSimpleProject()
{
    QUrl projecturl = QUrl::fromLocalFile( PROJECTS_SOURCE_DIR"/simpleproject/simpleproject.kdev4" );
    KDevSignalSpy projectSpy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    QVERIFY(projectSpy.wait(10000));
    IProject* project = ICore::self()->projectController()->findProjectByName( QStringLiteral("SimpleProject") );
    QVERIFY( project );

    QCOMPARE( project->buildSystemManager()->buildDirectory( project->projectItem() ),
              Path( "file:///home/andreas/projects/testcustom/build/" ) );
}

void TestCustomBuildSystemPlugin::buildDirProject()
{
    QUrl projecturl = QUrl::fromLocalFile( PROJECTS_SOURCE_DIR"/builddirproject/builddirproject.kdev4" );
    KDevSignalSpy projectSpy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    QVERIFY(projectSpy.wait(10000));
    IProject* project = ICore::self()->projectController()->findProjectByName( QStringLiteral("BuilddirProject") );
    QVERIFY( project );

    Path currentBuilddir = project->buildSystemManager()->buildDirectory( project->projectItem() );

    QCOMPARE( currentBuilddir, Path( projecturl ).parent() );
}


void TestCustomBuildSystemPlugin::loadMultiPathProject()
{
    QUrl projecturl = QUrl::fromLocalFile( PROJECTS_SOURCE_DIR"/multipathproject/multipathproject.kdev4" );
    KDevSignalSpy projectSpy(ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject( projecturl );
    // Wait for the project to be opened
    QVERIFY(projectSpy.wait(10000));
    IProject* project = ICore::self()->projectController()->findProjectByName( QStringLiteral("MultiPathProject") );
    QVERIFY( project );
    KDevelop::ProjectBaseItem* mainfile = nullptr;
    const auto& files = project->fileSet();
    for (const auto& file : files) {
        const auto& filesForPath = project->filesForPath(file);
        for (auto i: filesForPath) {
            if( i->text() == QLatin1String("main.cpp") ) {
                mainfile = i;
                break;
            }
        }
    }
    QVERIFY(mainfile);

    QCOMPARE( project->buildSystemManager()->buildDirectory( mainfile ),
              Path( "file:///home/andreas/projects/testcustom/build2/src" ) );
}

