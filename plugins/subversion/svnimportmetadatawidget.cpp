/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "svnimportmetadatawidget.h"
#include "ui_importmetadatawidget.h"
#include <vcs/vcslocation.h>

SvnImportMetadataWidget::SvnImportMetadataWidget( QWidget *parent )
    : VcsImportMetadataWidget( parent ), m_ui(new Ui::SvnImportMetadataWidget)
    , useSourceDirForDestination( false )
{
    m_ui->setupUi( this );
    m_ui->srcEdit->setUrl( QUrl() );
    connect( m_ui->srcEdit, &KUrlRequester::textChanged, this, &KDevelop::VcsImportMetadataWidget::changed );
    connect( m_ui->srcEdit, &KUrlRequester::urlSelected, this, &KDevelop::VcsImportMetadataWidget::changed );
    connect( m_ui->dest, &QLineEdit::textChanged, this, &KDevelop::VcsImportMetadataWidget::changed );
    connect( m_ui->message, &QTextEdit::textChanged, this, &KDevelop::VcsImportMetadataWidget::changed );
}

SvnImportMetadataWidget::~SvnImportMetadataWidget()
{
    delete m_ui;
}

void SvnImportMetadataWidget::setSourceLocation( const KDevelop::VcsLocation& importdir )
{
    m_ui->srcEdit->setUrl( importdir.localUrl() );
}

QUrl SvnImportMetadataWidget::source() const
{
    return m_ui->srcEdit->url();
}

KDevelop::VcsLocation SvnImportMetadataWidget::destination() const
{
    KDevelop::VcsLocation destloc;
    QString url = m_ui->dest->text();
    if( useSourceDirForDestination ) {
        url += QLatin1Char('/') + m_ui->srcEdit->url().fileName();
    }
    destloc.setRepositoryServer(url);
    return destloc;
}

void SvnImportMetadataWidget::setUseSourceDirForDestination( bool b )
{
    useSourceDirForDestination = b;
}


void SvnImportMetadataWidget::setSourceLocationEditable( bool enable )
{
    m_ui->srcEdit->setEnabled( enable );
}

void SvnImportMetadataWidget::setMessage(const QString& message)
{
    m_ui->message->setText(message);
}

QString SvnImportMetadataWidget::message() const
{
    return m_ui->message->toPlainText();
}

bool SvnImportMetadataWidget::hasValidData() const
{
    return !m_ui->message->toPlainText().isEmpty() && !m_ui->srcEdit->text().isEmpty();
}


