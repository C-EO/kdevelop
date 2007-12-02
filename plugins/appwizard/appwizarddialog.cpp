/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "appwizarddialog.h"

#include <klocale.h>

AppWizardDialog::AppWizardDialog(QWidget *parent, Qt::WFlags flags)
    :KAssistantDialog(parent, flags)
{
    setWindowTitle(i18n("Create New Project"));
    KDialog::showButton(Help, false);
//     KDialog::showButton(User2, false);
//     KDialog::showButton(User3, false);
}

