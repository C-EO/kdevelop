/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lldbconfigpage.h"
#include "ui_lldbconfigpage.h"

#include "dbgglobal.h"

#include <util/environmentselectionwidget.h>

#include <KConfigGroup>
#include <KUrlRequester>

#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace KDevelop;
namespace Config = KDevMI::LLDB::Config;

LldbConfigPage::LldbConfigPage(QWidget* parent)
    : LaunchConfigurationPage(parent)
    , ui(new Ui::LldbConfigPage)
{
    ui->setupUi(this);
    ui->lineDebuggerExec->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
    ui->lineConfigScript->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);

    QRegularExpression rx(QStringLiteral(R"([^:]+:\d+)"));
    ui->lineRemoteServer->setValidator(new QRegularExpressionValidator(rx, this));

    ui->comboStartWith->setItemData(0, QStringLiteral("ApplicationOutput"));
    ui->comboStartWith->setItemData(1, QStringLiteral("GdbConsole"));
    ui->comboStartWith->setItemData(2, QStringLiteral("FrameStack"));

    connect(ui->lineDebuggerExec, &KUrlRequester::textChanged, this, &LldbConfigPage::changed);
    connect(ui->lineDebuggerArgs, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
    connect(ui->comboEnv, &EnvironmentSelectionWidget::currentProfileChanged,
            this, &LldbConfigPage::changed);

    connect(ui->lineConfigScript, &KUrlRequester::textChanged, this, &LldbConfigPage::changed);
    connect(ui->comboStartWith, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LldbConfigPage::changed);

    connect(ui->groupRemote, &QGroupBox::clicked, this, &LldbConfigPage::changed);
    connect(ui->lineRemoteServer, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
    connect(ui->lineOnDevPath, &QLineEdit::textChanged, this, &LldbConfigPage::changed);
}

LldbConfigPage::~LldbConfigPage()
{
    delete ui;
}

QIcon LldbConfigPage::icon() const
{
    return {};
}

QString LldbConfigPage::title() const
{
    return i18nc("@title:tab", "LLDB Configuration");
}

void LldbConfigPage::loadFromConfiguration(const KConfigGroup& cfg, KDevelop::IProject *)
{
    bool block = blockSignals(true);
    ui->lineDebuggerExec->setUrl(cfg.readEntry(Config::LldbExecutableEntry, QUrl()));
    ui->lineDebuggerArgs->setText(cfg.readEntry(Config::LldbArgumentsEntry, QString()));
    ui->comboEnv->setCurrentProfile(cfg.readEntry(Config::LldbEnvironmentEntry, QString()));
    ui->checkInheritSystem->setChecked(cfg.readEntry(Config::LldbInheritSystemEnvEntry, true));
    ui->lineConfigScript->setUrl(cfg.readEntry(Config::LldbConfigScriptEntry, QUrl()));
    ui->checkBreakOnStart->setChecked(cfg.readEntry(KDevMI::Config::BreakOnStartEntry, false));
    ui->comboStartWith->setCurrentIndex(ui->comboStartWith->findData(
        cfg.readEntry(KDevMI::Config::StartWithEntry, "ApplicationOutput")));
    ui->groupRemote->setChecked(cfg.readEntry(Config::LldbRemoteDebuggingEntry, false));
    ui->lineRemoteServer->setText(cfg.readEntry(Config::LldbRemoteServerEntry, QString()));
    ui->lineOnDevPath->setText(cfg.readEntry(Config::LldbRemotePathEntry, QString()));
    blockSignals(block);
}

void LldbConfigPage::saveToConfiguration(KConfigGroup cfg, KDevelop::IProject *) const
{
    cfg.writeEntry(Config::LldbExecutableEntry, ui->lineDebuggerExec->url());
    cfg.writeEntry(Config::LldbArgumentsEntry, ui->lineDebuggerArgs->text());
    cfg.writeEntry(Config::LldbEnvironmentEntry, ui->comboEnv->currentProfile());
    cfg.writeEntry(Config::LldbInheritSystemEnvEntry, ui->checkInheritSystem->isChecked());
    cfg.writeEntry(Config::LldbConfigScriptEntry, ui->lineConfigScript->url());
    cfg.writeEntry(KDevMI::Config::BreakOnStartEntry, ui->checkBreakOnStart->isChecked());
    cfg.writeEntry(KDevMI::Config::StartWithEntry, ui->comboStartWith->currentData().toString());
    cfg.writeEntry(Config::LldbRemoteDebuggingEntry, ui->groupRemote->isChecked());
    cfg.writeEntry(Config::LldbRemoteServerEntry, ui->lineRemoteServer->text());
    cfg.writeEntry(Config::LldbRemotePathEntry, ui->lineOnDevPath->text());
}

KDevelop::LaunchConfigurationPage * LldbConfigPageFactory::createWidget(QWidget* parent)
{
    return new LldbConfigPage(parent);
}
