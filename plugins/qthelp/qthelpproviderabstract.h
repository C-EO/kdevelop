/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELPPROVIDERABSTRACT_H
#define QTHELPPROVIDERABSTRACT_H

#include <interfaces/idocumentationprovider.h>
#include <QObject>
#include <QHelpEngine>

#include <functional>

class QtHelpProviderAbstract : public QObject, public KDevelop::IDocumentationProvider
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IDocumentationProvider )
public:
    static QString collectionFileLocation();

    QtHelpProviderAbstract(QObject* parent, const QString& collectionFileName);
    ~QtHelpProviderAbstract() override;
    KDevelop::IDocumentation::Ptr documentationForDeclaration (KDevelop::Declaration*) const override;
    KDevelop::IDocumentation::Ptr documentation(const QUrl& url) const override;

    KDevelop::IDocumentation::Ptr documentationForIndex(const QModelIndex& idx) const override;
    QAbstractItemModel* indexModel() const override;

    KDevelop::IDocumentation::Ptr homePage() const override;

    /// @return False in case we failed to load any documentation files, else true
    bool isValid() const;

    QHelpEngine* engine() { return &m_engine; }
    const QHelpEngine* engine() const
    {
        return &m_engine;
    }

public Q_SLOTS:
    void jumpedTo(const QUrl& newUrl);

protected:
    /**
     * Calls @a m_engine.registerDocumentation(@p documentationFileName) and handles errors.
     */
    void registerDocumentation(const QString& documentationFileName);

    /**
     * Calls @a m_engine.unregisterDocumentation(@p namespaceName) and handles errors.
     */
    void unregisterDocumentation(const QString& namespaceName);

    /**
     * A function object that returns @c true if the namespace with a given name should be unregistered.
     */
    using ShouldUnregisterNamespace = std::function<bool(const QString& namespaceName)>;

    /**
     * Iterates over all registered namespaces and unregisters those indicated by @p shouldUnregisterNamespace.
     */
    void cleanUpRegisteredDocumentations(const ShouldUnregisterNamespace& shouldUnregisterNamespace);

    QHelpEngine m_engine;
};

#endif // QTHELPPROVIDERABSTRACT_H
