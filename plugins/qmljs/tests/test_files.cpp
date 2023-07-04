/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_files.h"

#include <language/duchain/duchain.h>
#include <language/duchain/problem.h>
#include <language/codegen/coderepresentation.h>
#include <language/backgroundparser/backgroundparser.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/json/declarationvalidator.h>

#include "testfilepaths.h"

//Include all used json tests, otherwise "Test not found"
#include <tests/json/jsondeclarationtests.h>
#include <tests/json/jsonducontexttests.h>
#include <tests/json/jsontypetests.h>
#include <interfaces/ilanguagecontroller.h>

#include <QTest>

// #include "cppjsontests.h"

using namespace KDevelop;

QTEST_MAIN(TestFiles)

void TestFiles::initTestCase()
{
  AutoTestShell::init({"kdevqmljs"});
  TestCore::initialize(KDevelop::Core::NoUi);
  DUChain::self()->disablePersistentStorage();
  Core::self()->languageController()->backgroundParser()->setDelay(0);
  CodeRepresentation::setDiskChangesForbidden(true);
}

void TestFiles::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestFiles::testQMLCustomComponent()
{
    // First parse CustomComponent, so that it is visible and can be used
    // by CustomComponentUser. Then re-parse CustomComponent and assert that
    // it has been used.
    parseAndCheck(TEST_FILES_DIR "/custom_component/CustomComponent.qml", false);
    parseAndCheck(TEST_FILES_DIR "/custom_component/CustomComponentUser.qml");
    parseAndCheck(TEST_FILES_DIR "/custom_component/CustomComponent.qml");
}

void TestFiles::testQMLTypes()
{
    parseAndCheck(TEST_FILES_DIR "/qmltypes/AnItem.qml", true);
}

void TestFiles::testTypeMismatchFalsePositives()
{
    parseAndCheck(TEST_FILES_DIR "/type_mismatch_false_positives/code.js", true);
}

void TestFiles::testJSUsesBetweenFiles()
{
    parseAndCheck(TEST_FILES_DIR "/js_cross_file_uses/js_variable_definition.js", false);
    parseAndCheck(TEST_FILES_DIR "/js_cross_file_uses/js_variable_use.js");
    parseAndCheck(TEST_FILES_DIR "/js_cross_file_uses/js_variable_definition.js");
}

void TestFiles::testNodeJS()
{
    parseAndCheck(TEST_FILES_DIR "/node_modules/module.js", false); // Ensure that module.js is in the DUChain
    parseAndCheck(TEST_FILES_DIR "/node.js/module2.js", false);
    parseAndCheck(TEST_FILES_DIR "/node.js/main.js");
}

void TestFiles::testFiles_data()
{
  QTest::addColumn<QString>("fileName");
  const QString testDirPath = TEST_FILES_DIR;
  const QStringList files = QDir(testDirPath).entryList(QStringList() << QStringLiteral("*.js") << QStringLiteral("*.qml"), QDir::Files);
  for (const QString& file : files) {
    QTest::newRow(file.toUtf8()) << QString(testDirPath + "/" + file);
  }
}

void TestFiles::testFiles()
{
  QFETCH(QString, fileName);
  parseAndCheck(fileName);
}

void TestFiles::parseAndCheck(const QString& fileName, bool check)
{
  const IndexedString indexedFileName(fileName);
  ReferencedTopDUContext top =
      DUChain::self()->waitForUpdate(indexedFileName, KDevelop::TopDUContext::AllDeclarationsContextsAndUses);

  while (!ICore::self()->languageController()->backgroundParser()->isIdle()) {
      QTest::qWait(500);
  }

  QVERIFY(top);

  if (check) {
    DUChainReadLocker lock;
    DeclarationValidator validator;
    top->visit(validator);
    QVERIFY(validator.testsPassed());

    if (!top->problems().isEmpty()) {
        foreach(auto p, top->problems()) {
            qDebug() << p;
        }
    }
    if (!QTest::currentDataTag() || strcmp("failparse.js", QTest::currentDataTag()) != 0) {
        QEXPECT_FAIL("plugins.qml", "not working properly yet", Continue);
        QEXPECT_FAIL("qrc_import.qml", "just making sure it does not crash", Continue);
        QVERIFY(top->problems().isEmpty());
    }
  }
}

#include "moc_test_files.cpp"
