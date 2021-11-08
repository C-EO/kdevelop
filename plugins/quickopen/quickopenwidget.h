/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2016 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_QUICKOPENWIDGET_H
#define KDEVPLATFORM_PLUGIN_QUICKOPENWIDGET_H

#include "ui_quickopenwidget.h"

#include <QMenu>
#include <QTime>
#include <QTimer>

class QuickOpenModel;

class QAbstractProxyModel;
class QLineEdit;

/// Will delete itself once the dialog is closed, so use QPointer when referencing it permanently
class QuickOpenWidget
    : public QMenu
{
    Q_OBJECT
public:
    /**
     * @param initialItems List of items that should initially be enabled in the quickopen-list. If empty, all are enabled.
     * @param initialScopes List of scopes that should initially be enabled in the quickopen-list. If empty, all are enabled.
     * @param listOnly when this is true, the given items will be listed, but all filtering using checkboxes is disabled.
     * @param noSearchField when this is true, no search-line is shown.
     * */
    QuickOpenWidget(QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly = false, bool noSearchField = false);
    ~QuickOpenWidget() override;
    void setPreselectedText(const QString& text);
    void prepareShow();

    void setAlternativeSearchField(QLineEdit* alterantiveSearchField);

    bool sortingEnabled() const;
    void setSortingEnabled(bool enabled);

    //Shows OK + Cancel. By default they are hidden
    void showStandardButtons(bool show);
    void showSearchField(bool show);
Q_SIGNALS:
    void scopesChanged(const QStringList& scopes);
    void itemsChanged(const QStringList& scopes);
    void ready();
private Q_SLOTS:
    void callRowSelected();

    void updateTimerInterval(bool cheapFilterChange);

    void accept();
    void textChanged(const QString& str);
    void updateProviders();
    void doubleClicked (const QModelIndex& index);

    void applyFilter();
private:
    void showEvent(QShowEvent*) override;

    bool eventFilter (QObject* watched, QEvent* event) override;

    void avoidMenuAltFocus();

    QuickOpenModel* m_model;
    QAbstractProxyModel* m_proxy = nullptr;
    bool m_sortingEnabled = false;
    bool m_expandedTemporary, m_hadNoCommandSinceAlt;
    QTime m_altDownTime;
    QString m_preselectedText;
    QTimer m_filterTimer;
    QString m_filter;
public:
    Ui::QuickOpenWidget ui;

    friend class QuickOpenWidgetDialog;
    friend class QuickOpenPlugin;
    friend class QuickOpenLineEdit;
};

class QuickOpenWidgetDialog
    : public QObject
{
    Q_OBJECT
public:
    QuickOpenWidgetDialog(const QString& title, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly = false, bool noSearchField = false);
    ~QuickOpenWidgetDialog() override;

    /// Shows the dialog
    void run();

    QuickOpenWidget* widget() const
    {
        return m_widget;
    }
private:
    QDialog* m_dialog; /// @warning m_dialog is also the parent
    QuickOpenWidget* m_widget;
};

#endif
