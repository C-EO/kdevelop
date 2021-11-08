/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_duchainshutdown.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <QTest>

using namespace KDevelop;

void TestDUChainShutdown::initTestCase()
{
    AutoTestShell::init();
    m_core = TestCore::initialize(Core::NoUi);
}

void TestDUChainShutdown::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDUChainShutdown::runTest()
{
    const QString ctxId = QStringLiteral("foo::bar::asdf");
    const QString path = QStringLiteral("/foo/myurl");
    const QString myLang = QStringLiteral("fooLang");

    // step 1, store a bunch of data in the repository
    IndexedTopDUContext idxTop;
    IndexedDUContext idxCtx;
    {
        DUChainWriteLocker lock;

        auto file = new ParsingEnvironmentFile(IndexedString(path));
        file->setLanguage(IndexedString(myLang));

        ReferencedTopDUContext top(new TopDUContext(IndexedString(path), RangeInRevision(1, 2, 3, 4), file));
        DUChain::self()->addDocumentChain(top);
        idxTop = IndexedTopDUContext(top);
        QVERIFY(idxTop.isValid());
        QVERIFY(idxTop.isLoaded());

        auto ctx = new DUContext(RangeInRevision(1, 2, 2, 3), top);
        ctx->setLocalScopeIdentifier(QualifiedIdentifier(ctxId));
        QCOMPARE(top->childContexts().size(), 1);

        idxCtx = IndexedDUContext(ctx);
        QVERIFY(idxCtx.isValid());
    }

    // shutdown and reinitialize - this should not crash :)
    m_core->setShuttingDown(true);
    DUChain::self()->shutdown();
    m_core->setShuttingDown(false);
    DUChain::self()->initialize();

    {
        DUChainReadLocker lock;
        // now verify that the data was properly restored
        QVERIFY(!idxTop.isLoaded());
        QVERIFY(idxTop.isValid());

        ReferencedTopDUContext top(idxTop.data());
        QVERIFY(top);
        QVERIFY(idxTop.isLoaded());
        QCOMPARE(top->childContexts().size(), 1);
        QCOMPARE(top->childContexts().first()->localScopeIdentifier().toString(), QStringLiteral("foo::bar::asdf"));
        QCOMPARE(idxCtx.data(), top->childContexts().first());
    }
    {
        DUChainWriteLocker lock;
        DUChain::self()->removeDocumentChain(idxTop.data());
    }
}

QTEST_MAIN(TestDUChainShutdown)
