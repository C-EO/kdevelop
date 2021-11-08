/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "coderepresentation.h"

#include <QFile>
#include <KTextEditor/Document>

#include <serialization/indexedstring.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>
#include <editor/modificationrevision.h>

namespace KDevelop {
static bool onDiskChangesForbidden = false;

QString CodeRepresentation::rangeText(const KTextEditor::Range& range) const
{
    Q_ASSERT(range.end().line() < lines());

    //Easier for single line ranges which should happen most of the time
    if (range.onSingleLine())
        return QString(line(range.start().line()).mid(range.start().column(), range.columnWidth()));

    //Add up al the requested lines
    QString rangedText = line(range.start().line()).mid(range.start().column());

    for (int i = range.start().line() + 1; i <= range.end().line(); ++i)
        rangedText += QLatin1Char('\n') + ((i == range.end().line()) ? line(i).left(range.end().column()) : line(i));

    return rangedText;
}

static void grepLine(const QString& identifier, const QString& lineText, int lineNumber,
                     QVector<KTextEditor::Range>& ret, bool surroundedByBoundary)
{
    if (identifier.isEmpty())
        return;

    int pos = 0;
    while (true) {
        pos = lineText.indexOf(identifier, pos);
        if (pos == -1)
            break;
        int start = pos;
        pos += identifier.length();
        int end = pos;

        if (!surroundedByBoundary ||
            ((end == lineText.length() || !lineText[end].isLetterOrNumber() || lineText[end] != QLatin1Char('_'))
             && (start - 1 < 0 || !lineText[start - 1].isLetterOrNumber() ||
                 lineText[start - 1] != QLatin1Char('_')))) {
            ret << KTextEditor::Range(lineNumber, start, lineNumber, end);
        }
    }
}

class EditorCodeRepresentation
    : public DynamicCodeRepresentation
{
public:
    explicit EditorCodeRepresentation(KTextEditor::Document* document) : m_document(document)
    {
        m_url = IndexedString(m_document->url());
    }

    QVector<KTextEditor::Range> grep(const QString& identifier, bool surroundedByBoundary) const override
    {
        QVector<KTextEditor::Range> ret;

        if (identifier.isEmpty())
            return ret;

        for (int line = 0; line < m_document->lines(); ++line)
            grepLine(identifier, m_document->line(line), line, ret, surroundedByBoundary);

        return ret;
    }

    KDevEditingTransaction::Ptr makeEditTransaction() override
    {
        return KDevEditingTransaction::Ptr(new KDevEditingTransaction(m_document));
    }

    QString line(int line) const override
    {
        if (line < 0 || line >= m_document->lines())
            return QString();
        return m_document->line(line);
    }

    int lines() const override
    {
        return m_document->lines();
    }

    QString text() const override
    {
        return m_document->text();
    }

    bool setText(const QString& text) override
    {
        bool ret;
        {
            KDevEditingTransaction t(m_document);
            ret = m_document->setText(text);
        }
        ModificationRevision::clearModificationCache(m_url);
        return ret;
    }

    bool fileExists() const override
    {
        return QFile(m_document->url().path()).exists();
    }

    bool replace(const KTextEditor::Range& range, const QString& oldText,
                 const QString& newText, bool ignoreOldText) override
    {
        QString old = m_document->text(range);
        if (oldText != old && !ignoreOldText) {
            return false;
        }

        bool ret;
        {
            KDevEditingTransaction t(m_document);
            ret = m_document->replaceText(range, newText);
        }

        ModificationRevision::clearModificationCache(m_url);

        return ret;
    }

    QString rangeText(const KTextEditor::Range& range) const override
    {
        return m_document->text(range);
    }

private:
    KTextEditor::Document* m_document;
    IndexedString m_url;
};

class FileCodeRepresentation
    : public CodeRepresentation
{
public:
    explicit FileCodeRepresentation(const IndexedString& document) : m_document(document)
    {
        QString localFile(document.toUrl().toLocalFile());

        QFile file(localFile);
        if (file.open(QIODevice::ReadOnly)) {
            data = QString::fromLocal8Bit(file.readAll());
            lineData = data.split(QLatin1Char('\n'));
        }
        m_exists = file.exists();
    }

    QString line(int line) const override
    {
        if (line < 0 || line >= lineData.size())
            return QString();

        return lineData.at(line);
    }

    QVector<KTextEditor::Range> grep(const QString& identifier, bool surroundedByBoundary) const override
    {
        QVector<KTextEditor::Range> ret;

        if (identifier.isEmpty())
            return ret;

        for (int line = 0; line < lineData.count(); ++line)
            grepLine(identifier, lineData.at(line), line, ret, surroundedByBoundary);

        return ret;
    }

    int lines() const override
    {
        return lineData.count();
    }

    QString text() const override
    {
        return data;
    }

    bool setText(const QString& text) override
    {
        Q_ASSERT(!onDiskChangesForbidden);
        QString localFile(m_document.toUrl().toLocalFile());

        QFile file(localFile);
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray data = text.toLocal8Bit();

            if (file.write(data) == data.size()) {
                ModificationRevision::clearModificationCache(m_document);
                return true;
            }
        }
        return false;
    }

    bool fileExists() const override
    {
        return m_exists;
    }

private:
    //We use QByteArray, because the column-numbers are measured in utf-8
    IndexedString m_document;
    bool m_exists;
    QStringList lineData;
    QString data;
};

class ArtificialStringData
    : public QSharedData
{
public:
    explicit ArtificialStringData(const QString& data)
    {
        setData(data);
    }
    void setData(const QString& data)
    {
        m_data = data;
        m_lineData = m_data.split(QLatin1Char('\n'));
    }
    QString data() const
    {
        return m_data;
    }
    const QStringList& lines() const
    {
        return m_lineData;
    }

private:
    QString m_data;
    QStringList m_lineData;
};

class StringCodeRepresentation
    : public CodeRepresentation
{
public:
    explicit StringCodeRepresentation(const QExplicitlySharedDataPointer<ArtificialStringData>& _data)
        : data(_data)
    {
        Q_ASSERT(data);
    }

    QString line(int line) const override
    {
        if (line < 0 || line >= data->lines().size())
            return QString();

        return data->lines().at(line);
    }

    int lines() const override
    {
        return data->lines().count();
    }

    QString text() const override
    {
        return data->data();
    }

    bool setText(const QString& text) override
    {
        data->setData(text);
        return true;
    }

    bool fileExists() const override
    {
        return false;
    }

    QVector<KTextEditor::Range> grep(const QString& identifier, bool surroundedByBoundary) const override
    {
        QVector<KTextEditor::Range> ret;

        if (identifier.isEmpty())
            return ret;

        for (int line = 0; line < data->lines().count(); ++line)
            grepLine(identifier, data->lines().at(line), line, ret, surroundedByBoundary);

        return ret;
    }

private:
    QExplicitlySharedDataPointer<ArtificialStringData> data;
};

static QHash<IndexedString, QExplicitlySharedDataPointer<ArtificialStringData>> artificialStrings;

//Return the representation for the given URL if it exists, or an empty pointer otherwise
static QExplicitlySharedDataPointer<ArtificialStringData> representationForPath(const IndexedString& path)
{
    const auto artificialStringIt = artificialStrings.constFind(path);
    if (artificialStringIt != artificialStrings.constEnd())
        return *artificialStringIt;
    else
    {
        IndexedString constructedPath(CodeRepresentation::artificialPath(path.str()));
        const auto artificialStringIt = artificialStrings.constFind(constructedPath);
        if (artificialStringIt != artificialStrings.constEnd())
            return *artificialStringIt;
        else
            return QExplicitlySharedDataPointer<ArtificialStringData>();
    }
}

bool artificialCodeRepresentationExists(const IndexedString& path)
{
    return representationForPath(path);
}

CodeRepresentation::Ptr createCodeRepresentation(const IndexedString& path)
{
    if (artificialCodeRepresentationExists(path))
        return CodeRepresentation::Ptr(new StringCodeRepresentation(representationForPath(path)));

    IDocument* document = ICore::self()->documentController()->documentForUrl(path.toUrl());
    if (document && document->textDocument())
        return CodeRepresentation::Ptr(new EditorCodeRepresentation(document->textDocument()));
    else
        return CodeRepresentation::Ptr(new FileCodeRepresentation(path));
}

void CodeRepresentation::setDiskChangesForbidden(bool changesForbidden)
{
    onDiskChangesForbidden = changesForbidden;
}

QString CodeRepresentation::artificialPath(const QString& name)
{
    QUrl url = QUrl::fromLocalFile(name);
    return QLatin1String("/kdev-artificial/") + url.adjusted(QUrl::NormalizePathSegments).path();
}

InsertArtificialCodeRepresentation::InsertArtificialCodeRepresentation(const IndexedString& file,
                                                                       const QString& text)
    : m_file(file)
{
    // make it simpler to use this by converting relative strings into artificial paths
    if (QUrl(m_file.str()).isRelative()) {
        m_file = IndexedString(CodeRepresentation::artificialPath(file.str()));

        int idx = 0;
        while (artificialStrings.contains(m_file)) {
            ++idx;
            m_file =
                IndexedString(CodeRepresentation::artificialPath(QStringLiteral("%1_%2").arg(idx).arg(file.str())));
        }
    }

    Q_ASSERT(!artificialStrings.contains(m_file));

    artificialStrings.insert(m_file,
                             QExplicitlySharedDataPointer<ArtificialStringData>(new ArtificialStringData(text)));
}

IndexedString InsertArtificialCodeRepresentation::file()
{
    return m_file;
}

InsertArtificialCodeRepresentation::~InsertArtificialCodeRepresentation()
{
    Q_ASSERT(artificialStrings.contains(m_file));
    artificialStrings.remove(m_file);
}

void InsertArtificialCodeRepresentation::setText(const QString& text)
{
    Q_ASSERT(artificialStrings.contains(m_file));
    artificialStrings[m_file]->setData(text);
}

QString InsertArtificialCodeRepresentation::text() const
{
    Q_ASSERT(artificialStrings.contains(m_file));
    return artificialStrings[m_file]->data();
}
}
