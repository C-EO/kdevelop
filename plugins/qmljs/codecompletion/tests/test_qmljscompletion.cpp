/*
    SPDX-FileCopyrightText: 2012 Sven Brauch <svenbrauch@googlemail.com>
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_qmljscompletion.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/codecompletion/codecompletioncontext.h>
#include <language/backgroundparser/backgroundparser.h>

#include <interfaces/ilanguagecontroller.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <QTest>

#include "../context.h"
#include "../model.h"

using namespace KDevelop;
using namespace QmlJS;

QTEST_MAIN(QmlJS::QmlCompletionTest)

using CompletionContextPtr = QSharedPointer<QmlJS::CodeCompletionContext>;

namespace {

struct CompletionParameters
{
    QSharedPointer<TestFile> file;
    DUContextPointer contextAtCursor;
    QString snip;
    QString remaining;
    CursorInRevision cursorAt;

    CompletionContextPtr completionContext;
    QList<CompletionTreeItemPointer> completionItems;
};

QStandardItemModel& fakeModel() {
  static QStandardItemModel model;
  model.setColumnCount(10);
  model.setRowCount(10);
  return model;
}


void runCompletion(CompletionParameters* parameters)
{
    parameters->completionContext = CompletionContextPtr(new QmlJS::CodeCompletionContext(parameters->contextAtCursor,
                                                               parameters->snip,
                                                               parameters->cursorAt));
    bool abort = false;

    parameters->completionItems = parameters->completionContext->completionItems(abort, true);
}

CompletionParameters prepareCompletion(const QString& initCode, const QString& invokeCode, bool qml)
{
    CompletionParameters completion_data;

    // Simulate that the user has entered invokeCode where %INVOKE is, put
    // the cursor where %CURSOR is, and then asked for completions
    QVERIFY_RETURN(initCode.indexOf("%INVOKE") != -1, completion_data);

    // Create a file containing the given code, with "%INVOKE" removed
    completion_data.file = QSharedPointer<TestFile>(new TestFile(QString(initCode).remove(QLatin1String("%INVOKE")),
                                                    qml ? "qml" : "js"));

    completion_data.file->parse();
    completion_data.file->waitForParsed();
    // wait for this fail and all dependencies, like modules and such
    while (!ICore::self()->languageController()->backgroundParser()->isIdle()) {
        QTest::qWait(100);
    }

    if (!completion_data.file->topContext()) {
      qWarning() << "file contents are: " << completion_data.file->fileContents();
      QVERIFY_RETURN(false, completion_data);
    }

    QString allCode = QString(initCode).replace(QLatin1String("%INVOKE"), invokeCode);

    QStringList lines = allCode.split('\n');
    completion_data.cursorAt = CursorInRevision::invalid();
    for ( int i = 0; i < lines.length(); i++ ) {
        int j = lines.at(i).indexOf(QLatin1String("%CURSOR"));
        if ( j != -1 ) {
            completion_data.cursorAt = CursorInRevision(i, j);
            break;
        }
    }
    QVERIFY_RETURN(completion_data.cursorAt.isValid(), completion_data);

    // codeCompletionContext only gets passed the text until the place where completion is invoked
    completion_data.snip = allCode.mid(0, allCode.indexOf(QLatin1String("%CURSOR")));
    completion_data.remaining = allCode.mid(allCode.indexOf(QLatin1String("%CURSOR")) + 7);

    DUChainReadLocker lock;
    completion_data.contextAtCursor = DUContextPointer(completion_data.file->topContext()->findContextAt(completion_data.cursorAt, true));
    QVERIFY_RETURN(completion_data.contextAtCursor, completion_data);

    runCompletion(&completion_data);

    return completion_data;
}

bool containsItemForDeclarationNamed(const CompletionParameters& params, const QString& itemName)
{
    DUChainReadLocker lock;

    foreach (const CompletionTreeItemPointer& ptr, params.completionItems) {
        if (ptr->declaration()) {
            if (ptr->declaration()->identifier().toString() == itemName) {
                return true;
            }
        }
    }

    return false;
}

bool containsItemForText(const CompletionParameters& params, const QString& item)
{
    QModelIndex idx = fakeModel().index(0, KDevelop::CodeCompletionModel::Name);

    for (auto ptr : params.completionItems) {
        if (ptr->data(idx, Qt::DisplayRole, nullptr).toString() == item) {
            return true;
        }
    }

    return false;
}

bool declarationInCompletionList(const QString& initCode, const QString& invokeCode, const QString& itemName, bool qml)
{
    return containsItemForDeclarationNamed(
        prepareCompletion(initCode, invokeCode, qml),
        itemName
    );
}

bool itemInCompletionList(const QString& initCode, const QString& invokeCode, const QString& itemName, bool qml)
{
    return containsItemForText(
        prepareCompletion(initCode, invokeCode, qml),
        itemName
    );
}

}

namespace QmlJS {

void QmlCompletionTest::initTestCase()
{
    AutoTestShell::init({"kdevqmljs"});
    TestCore::initialize(Core::NoUi);
    DUChain::self()->disablePersistentStorage();
    CodeRepresentation::setDiskChangesForbidden(true);
}

void QmlCompletionTest::cleanupTestCase()
{
  TestCore::shutdown();
}

void QmlCompletionTest::testContainsDeclaration()
{
    QFETCH(QString, invokeCode);
    QFETCH(QString, completionCode);
    QFETCH(QString, expectedItem);
    QFETCH(bool, qml);

    QVERIFY(declarationInCompletionList(invokeCode, completionCode, expectedItem, qml));
}

void QmlCompletionTest::testContainsDeclaration_data()
{
    QTest::addColumn<QString>("invokeCode");
    QTest::addColumn<QString>("completionCode");
    QTest::addColumn<QString>("expectedItem");
    QTest::addColumn<bool>("qml");

    // Comments and strings
    QTest::newRow("js_outside_single_line_comment") << "var a // this is a comment;\n%INVOKE" << "%CURSOR" << "a" << false;
    QTest::newRow("js_outside_multi_line_comment") << "var a;\n%INVOKE" << "/* comment */ %CURSOR" << "a" << false;
    QTest::newRow("js_outside_string") << "var a;\n%INVOKE" << "var b = 'hello' + %CURSOR" << "a" << false;

    // Basic JS tests
    QTest::newRow("js_basic_variable") << "var a;\n %INVOKE" << "%CURSOR" << "a" << false;
    QTest::newRow("js_basic_function") << "function f();\n %INVOKE" << "%CURSOR" << "f" << false;

    // Object members
    QTest::newRow("js_object_members") << "var a = {b: 0};\n %INVOKE" << "a.%CURSOR" << "b" << false;
    QTest::newRow("js_array_subscript") << "var a = {b: 0};\n %INVOKE" << "a[%CURSOR" << "b" << false;
    QTest::newRow("js_skip_separators") << "var a = {b: 0};\n %INVOKE" << "foo(false, a.%CURSOR" << "b" << false;

    // Javascript classes
    QTest::newRow("js_object") << "var o = {};\n %INVOKE" << "o.%CURSOR" << "toString" << false;
    QTest::newRow("js_builtin_object") << "var a;\n %INVOKE" << "a.%CURSOR" << "toString" << false;
    QTest::newRow("js_builtin_string") << "var a = '';\n %INVOKE" << "a.%CURSOR" << "split" << false;
    QTest::newRow("js_builtin_number") << "var a = 2;\n %INVOKE" << "a.%CURSOR" << "toFixed" << false;
    QTest::newRow("js_builtin_boolean") << "var a = true;\n %INVOKE" << "a.%CURSOR" << "valueOf" << false;
    QTest::newRow("js_builtin_array") << "var a = [];\n %INVOKE" << "a.%CURSOR" << "slice" << false;
    QTest::newRow("js_builtin_function") << "var a = function(){};\n %INVOKE" << "a.%CURSOR" << "apply" << false;
    QTest::newRow("js_builtin_regexp") << "var a = /.*/;\n %INVOKE" << "a.%CURSOR" << "exec" << false;
    QTest::newRow("js_dom_document") << "%INVOKE" << "%CURSOR" << "document" << false;

    // Basic QML tests
    QTest::newRow("qml_basic_property") << "Item { id: foo\n property int prop\n %INVOKE }" << "%CURSOR" << "prop" << true;
    QTest::newRow("qml_basic_instance") << "Item { id: foo\n %INVOKE }" << "onTest: %CURSOR" << "foo" << true;
    QTest::newRow("qml_skip_separators") << "Item { id: foo\n Item { id: bar\n property int prop }\n %INVOKE" << "onTest: bar.%CURSOR" << "prop" << true;

    // QML inheritance
    QTest::newRow("qml_inheritance") <<
        "Module {\n"
        " Component {\n"
        "  name: \"TestComponent\"\n"
        "  Property {\n"
        "   name: \"prop\"\n"
        "   type: \"int\"\n"
        "  }\n"
        " }\n"
        " TestComponent {\n"
        "  id: foo\n"
        "  %INVOKE\n"
        " }\n"
        "}" << "%CURSOR" << "prop" << true;

    // QML parent
    QTest::newRow("qml_parent") <<
        "Item {\n"
        " id: a\n"
        " property var prop\n"
        " Item {\n"
        "  id: b\n"
        "  %INVOKE\n"
        " }\n"
        "}\n" << "onTest: parent.%CURSOR" << "prop" << true;

    // This declaration must be in QtQuick 2.2 but not 2.0 (tested in testDoesNotContainDeclaration)
    QTest::newRow("qml_module_version_2.2") << "import QtQuick 2.2\n Item { id: a\n %INVOKE }" << "%CURSOR" << "OpacityAnimator" << true;
    QTest::newRow("qml_module_alias") << "import QtQuick 2.2 as Foo\n Item { id: a\n %INVOKE }" << "Foo.%CURSOR" << "OpacityAnimator" << true;

    // Built-in QML types
    QTest::newRow("qml_builtin_types") << "import QtQuick 2.0\n"
                                          "\n"
                                          "Text {\n"
                                          " id: foo\n"
                                          " %INVOKE\n"
                                          "}\n"
                                       << "font.%CURSOR"
                                       << "family" << true;
}

void QmlCompletionTest::testDoesNotContainDeclaration()
{
    QFETCH(QString, invokeCode);
    QFETCH(QString, completionCode);
    QFETCH(QString, item);
    QFETCH(bool, qml);

    QVERIFY(!declarationInCompletionList(invokeCode, completionCode, item, qml));
}

void QmlCompletionTest::testDoesNotContainDeclaration_data()
{
    QTest::addColumn<QString>("invokeCode");
    QTest::addColumn<QString>("completionCode");
    QTest::addColumn<QString>("item");
    QTest::addColumn<bool>("qml");

    // Comments and strings
    QTest::newRow("js_in_single_line_comment") << "var a;\n%INVOKE" << "// %CURSOR" << "a" << false;
    QTest::newRow("js_in_multi_line_comment") << "var a;\n%INVOKE" << "/* %CURSOR" << "a" << false;
    QTest::newRow("js_in_string") << "var a;\n%INVOKE" << "var b = 'hello \\'%CURSOR" << "a" << false;
    QTest::newRow("js_useless_completions") << "var a;\n%INVOKE" << "var %CURSOR" << "a" << false;
    QTest::newRow("js_useless_completions") << "var a;\n%INVOKE" << "function %CURSOR" << "a" << false;
    QTest::newRow("js_useless_completions") << "var a;\n%INVOKE" << "function f(%CURSOR" << "a" << false;
    QTest::newRow("js_useless_completions") << "var a;\n%INVOKE" << "var o = {id: %CURSOR" << "a" << false;

    // Don't show unreachable declarations when providing code-completions for object members
    QTest::newRow("js_object_member_not_surrounding") << "var a; var b = {c: 0};%INVOKE" << "b.%CURSOR" << "a" << false;
    QTest::newRow("js_object_member_local") << "var a = {b: 0};%INVOKE" << "%CURSOR" << "b" << false;

    // When providing completions for script bindings, don't propose script bindings
    // for properties/signals of the surrounding components
    QTest::newRow("qml_script_binding_not_surrounding") << "Item { property int foo; Item { %INVOKE } }" << "%CURSOR" << "foo" << false;

    // Don't list the declarations that are not in a namespace but are imported from it
    QTest::newRow("qml_namespace_js_builtins") << "import org.kde.kdevplatform 1.0 as KDev\n Item { id: a\n %INVOKE }" << "KDev.%CURSOR" << "String" << true;
}

void QmlCompletionTest::testContainsText()
{
    QFETCH(QString, invokeCode);
    QFETCH(QString, completionCode);
    QFETCH(QString, item);
    QFETCH(bool, qml);

    QVERIFY(itemInCompletionList(invokeCode, completionCode, item, qml));
}

void QmlCompletionTest::testContainsText_data()
{
    QTest::addColumn<QString>("invokeCode");
    QTest::addColumn<QString>("completionCode");
    QTest::addColumn<QString>("item");
    QTest::addColumn<bool>("qml");

    QTest::newRow("qml_import") << "%INVOKE" << "import %CURSOR" << "QtQuick" << true;
    QTest::newRow("qml_import_prefix") << "%INVOKE\nItem {}" << "import QtQuick.%CURSOR" << "QtQuick.Controls" << true;

    QTest::newRow("js_node_require") << "%INVOKE" << "var m = require(%CURSOR" << "tls" << false;
}


}
