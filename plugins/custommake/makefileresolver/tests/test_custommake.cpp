/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_custommake.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryDir>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../makefileresolver.h"

#include <QTest>

using namespace KDevelop;

namespace {
void createFile( QFile& file )
{
    file.remove();
    if ( !file.open( QIODevice::ReadWrite ) ) {
        qFatal("Cannot create the file %s", file.fileName().toUtf8().data());
    }
}
}

void TestCustomMake::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCustomMake::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCustomMake::testIncludeDirectories()
{
    QTemporaryDir tempDir;
    {
        QFile file( tempDir.path() + "/Makefile" );
        createFile( file );
        QFile testfile( tempDir.path() + "/testfile.cpp" );
        createFile(testfile);
        QTextStream stream1( &file );
        stream1 << "testfile.o:\n\t g++ testfile.cpp -I/testFile1 -I /testFile2 -isystem /testFile3 --include-dir=/testFile4 -o testfile";
    }

    MakeFileResolver mf;
    auto result = mf.resolveIncludePath(tempDir.path() + "/testfile.cpp");
    if (!result.success) {
      qDebug() << result.errorMessage << result.longErrorMessage;
      QFAIL("Failed to resolve include path.");
    }
    QCOMPARE(result.paths.size(), 4);
    QVERIFY(result.paths.contains(Path("/testFile1")));
    QVERIFY(result.paths.contains(Path("/testFile2")));
    QVERIFY(result.paths.contains(Path("/testFile3")));
    QVERIFY(result.paths.contains(Path("/testFile4")));
}

void TestCustomMake::testFrameworkDirectories()
{
    QTemporaryDir tempDir;
    int expectedPaths = 2;
    {
        QFile file( tempDir.path() + "/Makefile" );
        createFile( file );
        QFile testfile( tempDir.path() + "/testfile.cpp" );
        createFile(testfile);
        QTextStream stream1( &file );
        stream1 << "testfile.o:\n\t clang++ testfile.cpp -iframework /System/Library/Frameworks -F/Library/Frameworks -o testfile";
    }

    MakeFileResolver mf;
    auto result = mf.resolveIncludePath(tempDir.path() + "/testfile.cpp");
    if (!result.success) {
      qDebug() << result.errorMessage << result.longErrorMessage;
      QFAIL("Failed to resolve include path.");
    }
    QCOMPARE(result.frameworkDirectories.size(), expectedPaths);
    QVERIFY(result.frameworkDirectories.contains(Path("/System/Library/Frameworks")));
    QVERIFY(result.frameworkDirectories.contains(Path("/Library/Frameworks")));
}

void TestCustomMake::testDefines()
{
    MakeFileResolver mf;
    const auto result = mf.processOutput("-DFOO  -DFOO=\\\"foo\\\" -DBAR=ASDF -DLALA=1 -DMEH="
                                         " -DSTR=\"\\\"foo \\\\\\\" bar\\\"\" -DEND"
                                         " -DX=1 -UX", QString());
    QCOMPARE(result.defines.value("FOO", "not found"), QString("\"foo\""));
    QCOMPARE(result.defines.value("BAR", "not found"), QString("ASDF"));
    QCOMPARE(result.defines.value("LALA", "not found"), QString("1"));
    QCOMPARE(result.defines.value("MEH", "not found"), QString());
    QCOMPARE(result.defines.value("STR", "not found"), QString("\"foo \\\" bar\""));
    QCOMPARE(result.defines.value("END", "not found"), QString());
    QCOMPARE(result.defines.value("X", "not found"), QString("not found"));
}

QTEST_GUILESS_MAIN(TestCustomMake)

#include "moc_test_custommake.cpp"
