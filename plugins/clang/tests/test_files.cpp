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
#include "testprovider.h"
#include "duchain/clanghelpers.h"

//Include all used json tests, otherwise "Test not found"
#include <tests/json/jsondeclarationtests.h>
#include <tests/json/jsonducontexttests.h>
#include <tests/json/jsontypetests.h>
#include <interfaces/ilanguagecontroller.h>

#include <QDebug>
#include <QTest>
#include <QLoggingCategory>
#include <QProcess>
#include <QRegularExpression>
#include <QVersionNumber>

using namespace KDevelop;

QTEST_MAIN(TestFiles)

namespace {
bool isCudaAvailable()
{
    return QProcess::execute(QStringLiteral("clang"), {QStringLiteral("-xcuda"), QStringLiteral("-fsyntax-only"), QProcess::nullDevice()}) == 0;
}

QVersionNumber clangVersionNumber()
{
    return QVersionNumber::fromString(ClangHelpers::clangVersion());
}

QRegularExpression rangeRegularExpression()
{
    const auto capturedCursorRegexp = QStringLiteral("(\\(\\d+, \\d+\\))");
    const auto rangeRegexp = QStringLiteral("\\[%1, %2\\]").arg(capturedCursorRegexp, capturedCursorRegexp);
    return QRegularExpression{QRegularExpression::anchoredPattern(rangeRegexp)};
}

void expandTestFilesDir(QVariantMap& testData)
{
    const auto idIt = testData.find("identifier");
    if (idIt == testData.end()) {
        return; // nothing to adjust
    }

    static const QLatin1String testFilesDirVariableName("${TEST_FILES_DIR}");

    QCOMPARE(idIt->userType(), QMetaType::QString);
    auto identifier = idIt->toString();
    if (!identifier.contains(testFilesDirVariableName)) {
        return; // nothing to adjust
    }

    if (clangVersionNumber() >= QVersionNumber(13, 0, 0)) {
        *idIt = identifier.replace(testFilesDirVariableName, TEST_FILES_DIR);
        return; // done
    }

    // Older Clang versions return an empty identifier for unnamed struct and anonymous union.
    *idIt = QString();

    // KDevelop's Visitor::createDeclarationCommon() assigns the range's end to its start when
    // the identifier is empty, so the same is done below.
    const auto rangeIt = testData.find("range");
    if (rangeIt == testData.end()) {
        return; // no range => nothing left to adjust
    }

    QCOMPARE(rangeIt->userType(), QMetaType::QString);
    auto range = rangeIt->toString();

    static const auto regexp = rangeRegularExpression();
    QVERIFY(range.contains(regexp));
    *rangeIt = range.replace(regexp, "[\\1, \\1]");
}

/// libclang 16.0.0 and later does not qualify template arguments redundantly
/// and does not spell out default template argument values.
void adjustTemplateArguments(QVariantMap& testData)
{
    if (clangVersionNumber() < QVersionNumber(16, 0, 0)) {
        return; // nothing to adjust
    }

    const auto typeIt = testData.find("type");
    if (typeIt == testData.end()) {
        return; // nothing to adjust
    }

    QCOMPARE(typeIt->userType(), QMetaType::QVariantMap);
    auto typeData = typeIt->toMap();

    const auto toStringIt = typeData.find("toString");
    QVERIFY(toStringIt != typeData.end());
    QCOMPARE(toStringIt->userType(), QMetaType::QString);
    auto typeString = toStringIt->toString();

    const auto replaceInTypeString = [&](const char* before, const char* after, const char* replacementDescription) {
        if (!typeString.contains(QLatin1String{before}))
            return;
        typeString.replace(QLatin1String{before}, QLatin1String{after});
        qDebug() << replacementDescription << toStringIt->toString() << "=>" << typeString;
        *toStringIt = typeString;
        *typeIt = typeData;
    };

    replaceInTypeString("FormattingBug::X", "X", "Removing template argument qualification:");
    replaceInTypeString("SpacedDefaultParam< 20 >", "SpacedDefaultParam<  >",
                        "Unspecifying default template argument:");
}

void adjustTestData(QVariantMap& testData)
{
    expandTestFilesDir(testData);
    adjustTemplateArguments(testData);
}
} // unnamed namespace

void TestFiles::initTestCase()
{
    qputenv("KDEV_CLANG_JSON_TEST_RUN", "1");
    qputenv("KDEV_CLANG_EXTRA_ARGUMENTS", "-Wno-unused-variable -Wno-unused-parameter -Wno-unused-comparison -Wno-unused-value -Wno-unused-private-field -Wno-ignored-attributes");

    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));
    AutoTestShell::init({"kdevclangsupport"});
    TestCore::initialize(Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    Core::self()->languageController()->backgroundParser()->setDelay(0);
    CodeRepresentation::setDiskChangesForbidden(true);

    m_provider = new TestEnvironmentProvider;
    IDefinesAndIncludesManager::manager()->registerBackgroundProvider(m_provider);
}

void TestFiles::cleanupTestCase()
{
    delete m_provider;
    TestCore::shutdown();
}

void TestFiles::cleanup()
{
    m_provider->clear();
}

void TestFiles::testFiles_data()
{
    QTest::addColumn<QString>("fileName");
    const QString testDirPath = TEST_FILES_DIR;
    auto patterns = QStringList{"*.h", "*.cpp", "*.c", "*.cl"};
    if (isCudaAvailable()) {
        patterns.append("*.cu");
    }
    const QStringList files = QDir(testDirPath).entryList(patterns, QDir::Files);
    for (const QString& file : files) {
        QTest::newRow(file.toUtf8().constData()) << QString(testDirPath + '/' + file);
    }
}

void TestFiles::testFiles()
{
    QFETCH(QString, fileName);

    if (QTest::currentDataTag() == QLatin1String("lambdas.cpp")) {
        m_provider->parserArguments += "-std=c++14";
    } else if (QTest::currentDataTag() == QLatin1String("templates.cpp")) {
        m_provider->parserArguments += "-std=c++17";
    } else if (QTest::currentDataTag() == QLatin1String("types.cpp")) {
        if (clangVersionNumber() >= QVersionNumber(18, 0, 0)) {
            // without this flag, the top->problems().isEmpty() check below fails at the line `int arr2[argc];`
            m_provider->parserArguments += "-Wno-vla-cxx-extension";
        }
    }

    const IndexedString indexedFileName(fileName);
    ReferencedTopDUContext top =
        DUChain::self()->waitForUpdate(indexedFileName, TopDUContext::AllDeclarationsContextsAndUses);
    QVERIFY(top);
    DUChainReadLocker lock;
    DeclarationValidator validator(adjustTestData);
    top->visit(validator);

    const auto problems = top->problems();
    for (auto& problem : problems) {
        qDebug() << problem;
    }

    if (clangVersionNumber() < QVersionNumber(9, 0, 0))
        QEXPECT_FAIL("lambdas.cpp", "capture with identifier and initializer aren't visited apparently", Abort);
    QVERIFY(validator.testsPassed());

    if (!QTest::currentDataTag() || strcmp("invalid.cpp", QTest::currentDataTag()) != 0) {
        QVERIFY(top->problems().isEmpty());
    }
}

#include "moc_test_files.cpp"
