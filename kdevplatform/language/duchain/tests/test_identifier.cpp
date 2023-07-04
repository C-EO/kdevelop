/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_identifier.h"

#include <language/duchain/identifier.h>
#include <serialization/indexedstring.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <utility>
#include <QTest>

QTEST_GUILESS_MAIN(TestIdentifier)

using namespace KDevelop;

void TestIdentifier::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestIdentifier::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestIdentifier::testIdentifier()
{
    QFETCH(QString, stringId);
    const IndexedString indexedStringId(stringId);

    Identifier id(stringId);
    QCOMPARE(id.isEmpty(), stringId.isEmpty());
    QCOMPARE(id, Identifier(stringId));
    QVERIFY(!(id != Identifier(stringId)));
    QCOMPARE(id, Identifier(stringId));
    QCOMPARE(id, Identifier(indexedStringId));
    QCOMPARE(id.identifier(), indexedStringId);
    QCOMPARE(id.toString(), stringId);
    QVERIFY(id.nameEquals(Identifier(stringId)));
    QVERIFY(!id.isUnique());

    if (stringId.isEmpty()) {
        QVERIFY(id.inRepository());
        QVERIFY(Identifier(id).inRepository());
        QVERIFY(Identifier(indexedStringId).inRepository());
    }

    Identifier copy = id;
    QCOMPARE(copy, id);
    copy = copy;
    QCOMPARE(copy, id);
    copy = Identifier();
    QVERIFY(copy.isEmpty());
    copy = id;
    QCOMPARE(copy, id);

    IndexedIdentifier indexedId(id);
    QVERIFY(indexedId == id);
    QCOMPARE(indexedId, IndexedIdentifier(id));
    QCOMPARE(indexedId.isEmpty(), stringId.isEmpty());
    QCOMPARE(indexedId.identifier(), id);
    IndexedIdentifier indexedCopy = indexedId;
    QCOMPARE(indexedCopy, indexedId);
    indexedCopy = indexedCopy;
    QCOMPARE(indexedCopy, indexedId);
    indexedCopy = IndexedIdentifier();
    QVERIFY(indexedCopy.isEmpty());
    indexedCopy = indexedId;
    QCOMPARE(indexedCopy, indexedId);

    Identifier moved = std::move(id);
    QVERIFY(id.isEmpty());
    QVERIFY(id.inRepository());
    QCOMPARE(moved, copy);

    IndexedIdentifier movedIndexed = std::move(indexedId);
    QCOMPARE(movedIndexed, indexedCopy);
}

void TestIdentifier::testIdentifier_data()
{
    QTest::addColumn<QString>("stringId");

    QTest::newRow("empty") << QString();
    QTest::newRow("foo") << QStringLiteral("foo");
    QTest::newRow("bar") << QStringLiteral("bar");
    //TODO: test template identifiers
}

void TestIdentifier::testQualifiedIdentifier()
{
    QFETCH(QString, stringId);
    const QStringList list = stringId.split(QStringLiteral("::"), Qt::SkipEmptyParts);
    QualifiedIdentifier id(stringId);
    QCOMPARE(id.isEmpty(), stringId.isEmpty());
    QCOMPARE(id, QualifiedIdentifier(stringId));
    QVERIFY(!(id != QualifiedIdentifier(stringId)));
    QCOMPARE(id, QualifiedIdentifier(stringId));
    if (list.size() == 1) {
        QCOMPARE(id, QualifiedIdentifier(Identifier(list.last())));
    } else if (list.isEmpty()) {
        QualifiedIdentifier empty{Identifier()};
        QCOMPARE(id, empty);
        QVERIFY(empty.isEmpty());
        QVERIFY(empty.inRepository());
    }
    QCOMPARE(id.toString(), stringId);
    QCOMPARE(id.toStringList(), list);
    QCOMPARE(id.top(), Identifier(list.isEmpty() ? QString() : list.last()));

    if (stringId.isEmpty()) {
        QVERIFY(id.inRepository());
        QVERIFY(QualifiedIdentifier(id).inRepository());
    }

    QualifiedIdentifier copy = id;
    QCOMPARE(copy, id);
    copy = copy;
    QCOMPARE(copy, id);
    copy = QualifiedIdentifier();
    QVERIFY(copy.isEmpty());
    copy = id;
    QCOMPARE(copy, id);

    IndexedQualifiedIdentifier indexedId(id);
    QVERIFY(indexedId == id);
    QCOMPARE(indexedId, IndexedQualifiedIdentifier(id));
    QCOMPARE(indexedId.isValid(), !stringId.isEmpty());
    QCOMPARE(indexedId.identifier(), id);
    IndexedQualifiedIdentifier indexedCopy = indexedId;
    QCOMPARE(indexedCopy, indexedId);
    indexedCopy = indexedCopy;
    QCOMPARE(indexedCopy, indexedId);
    indexedCopy = IndexedQualifiedIdentifier();
    QVERIFY(!indexedCopy.isValid());
    indexedCopy = indexedId;
    QCOMPARE(indexedCopy, indexedId);

    QualifiedIdentifier moved = std::move(id);
    QVERIFY(id.isEmpty());
    QCOMPARE(moved, copy);

    IndexedQualifiedIdentifier movedIndexed = std::move(indexedId);
    QCOMPARE(movedIndexed, indexedCopy);

    copy.clear();
    QVERIFY(copy.isEmpty());

    copy.push(moved);
    QCOMPARE(copy, moved);

    copy.push(Identifier(QStringLiteral("lalala")));
    QCOMPARE(copy.count(), moved.count() + 1);
}

void TestIdentifier::testQualifiedIdentifier_data()
{
    QTest::addColumn<QString>("stringId");

    QTest::newRow("empty") << QString();
    QTest::newRow("foo") << "foo";
    QTest::newRow("foo::bar") << "foo::bar";
    //TODO: test template identifiers
}

void TestIdentifier::benchIdentifierCopyConstant()
{
    QBENCHMARK {
        Identifier identifier(QStringLiteral("Asdf"));
        identifier.index();
        Identifier copy(identifier);
    }
}

void TestIdentifier::benchIdentifierCopyDynamic()
{
    QBENCHMARK {
        Identifier identifier(QStringLiteral("Asdf"));
        Identifier copy(identifier);
    }
}

void TestIdentifier::benchQidCopyPush()
{
    QBENCHMARK {
        Identifier id(QStringLiteral("foo"));
        QualifiedIdentifier base(id);
        QualifiedIdentifier copy(base);
        copy.push(id);
    }
}

#include "moc_test_identifier.cpp"
