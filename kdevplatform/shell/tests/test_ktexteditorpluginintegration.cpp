/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_ktexteditorpluginintegration.h"

#include <QTest>
#include <QLoggingCategory>
#include <QSignalSpy>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <shell/plugincontroller.h>
#include <shell/uicontroller.h>

#include <KTextEditor/Application>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>
#include <KTextEditor/View>
#include <KTextEditor/Document>

using namespace KDevelop;

namespace {
template<typename T>
QPointer<T> makeQPointer(T *ptr)
{
    return {ptr};
}

IToolViewFactory *findToolView(const QString &id)
{
    const auto uiController = Core::self()->uiControllerInternal();
    const auto map = uiController->factoryDocuments();
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it.key()->id() == id) {
            return it.key();
        }
    }
    return nullptr;
}

class TestPlugin : public KTextEditor::Plugin
{
    Q_OBJECT
public:
    explicit TestPlugin(QObject *parent)
        : Plugin(parent)
    {
    }

    QObject *createView(KTextEditor::MainWindow * mainWindow) override
    {
        return new QObject(mainWindow);
    }
};
}

void TestKTextEditorPluginIntegration::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\n"));
    AutoTestShell::init({QStringLiteral("katesnippetsplugin")});
    TestCore::initialize();
    QVERIFY(KTextEditor::Editor::instance());
}

void TestKTextEditorPluginIntegration::cleanupTestCase()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));

    const auto editor = makeQPointer(KTextEditor::Editor::instance());
    const auto application = makeQPointer(editor->application());
    const auto window = makeQPointer(application->activeMainWindow());

    TestCore::shutdown();

    QVERIFY(!plugin);
    QVERIFY(!window);
    QVERIFY(!application);

    // editor lives by design until QCoreApplication terminates, then autodeletes
}

void TestKTextEditorPluginIntegration::testApplication()
{
    auto app = KTextEditor::Editor::instance()->application();
    QVERIFY(app);
    QVERIFY(app->parent());
    QCOMPARE(app->parent()->metaObject()->className(), "KTextEditorIntegration::Application");
    QVERIFY(app->activeMainWindow());
    QCOMPARE(app->mainWindows().size(), 1);
    QVERIFY(app->mainWindows().contains(app->activeMainWindow()));
}

void TestKTextEditorPluginIntegration::testMainWindow()
{
    auto window = KTextEditor::Editor::instance()->application()->activeMainWindow();
    QVERIFY(window);
    QVERIFY(window->parent());
    QCOMPARE(window->parent()->metaObject()->className(), "KTextEditorIntegration::MainWindow");

    const auto id = QStringLiteral("kte_integration_toolview");
    const auto icon = QIcon::fromTheme(QStringLiteral("kdevelop"));
    const auto text = QStringLiteral("some text");
    QVERIFY(!findToolView(id));

    auto plugin = new TestPlugin(this);
    auto toolView = makeQPointer(window->createToolView(plugin, id, KTextEditor::MainWindow::Bottom, icon, text));
    QVERIFY(toolView);

    auto factory = findToolView(id);
    QVERIFY(factory);

    // we reuse the same view
    QWidget parent;
    auto kdevToolView = makeQPointer(factory->create(&parent));
    QCOMPARE(kdevToolView->parentWidget(), &parent);
    QCOMPARE(toolView->parentWidget(), kdevToolView.data());

    // the children are kept alive when the tool view gets destroyed
    delete kdevToolView;
    QVERIFY(toolView);
    kdevToolView = factory->create(&parent);
    // and we reuse the ktexteditor tool view for the new kdevelop tool view
    QCOMPARE(toolView->parentWidget(), kdevToolView.data());

    delete toolView;
    delete kdevToolView;

    delete plugin;
    QVERIFY(!findToolView(id));
}

void TestKTextEditorPluginIntegration::testPlugin()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));
    if (!plugin) {
        QSKIP("Cannot continue without katesnippetsplugin, install Kate");
    }

    auto app = KTextEditor::Editor::instance()->application();
    auto ktePlugin = makeQPointer(app->plugin(id));
    QVERIFY(ktePlugin);

    auto view = makeQPointer(app->activeMainWindow()->pluginView(id));
    QVERIFY(view);
    const auto rawView = view.data();

    QSignalSpy spy(app->activeMainWindow(), &KTextEditor::MainWindow::pluginViewDeleted);
    QVERIFY(controller->unloadPlugin(id));
    QVERIFY(!ktePlugin);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().count(), 2);
    QCOMPARE(spy.first().at(0), QVariant::fromValue(id));
    QCOMPARE(spy.first().at(1), QVariant::fromValue(rawView));
    QVERIFY(!view);
}

void TestKTextEditorPluginIntegration::testPluginUnload()
{
    auto controller = Core::self()->pluginController();
    const auto id = QStringLiteral("katesnippetsplugin");
    auto plugin = makeQPointer(controller->loadPlugin(id));
    if (!plugin) {
        QSKIP("Cannot continue without katesnippetsplugin, install Kate");
    }

    auto app = KTextEditor::Editor::instance()->application();
    auto ktePlugin = makeQPointer(app->plugin(id));
    QVERIFY(ktePlugin);
    delete ktePlugin;
    // don't crash
    plugin->unload();
}

QTEST_MAIN(TestKTextEditorPluginIntegration)

#include <test_ktexteditorpluginintegration.moc>
