/*
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_OUTPUTPAGE_H
#define KDEVPLATFORM_PLUGIN_OUTPUTPAGE_H

#include <KTextEditor/Cursor>

#include <QWidget>
#include <QUrl>

#include "ipagefocus.h"

namespace KDevelop {

class TemplateRenderer;
class SourceFileTemplate;
class CreateClassAssistant;

/**
 * Assistant page for setting the output location of generated source code
 */
class OutputPage : public QWidget, public IPageFocus
{
    Q_OBJECT

public:
    explicit OutputPage(QWidget* parent);
    ~OutputPage() override;

    /**
     * Creates form widgets according to the number of output files of the template @p fileTemplate.
     * File identifiers and labels are read from @p fileTemplate, but not the actual URLs.
     *
     * This function is useful to prevent UI flickering that occurs when adding widgets while the page is visible.
     * It can be called immediately after the template is selected, before the user specified anything for the generated code.
     */
    void prepareForm(const KDevelop::SourceFileTemplate& fileTemplate);
    /**
     * Loads file URLs from the template @p fileTemplate.
     * This function only sets URLs and file positions to widgets created by prepareForm(),
     * so be sure to call prepareForm() before calling this function.
     *
     * @param fileTemplate the template archive with the generated files
     * @param baseUrl the directory where the files are to be generated
     * @param renderer used to render any variables in output URLs
     */
    void loadFileTemplate(const KDevelop::SourceFileTemplate& fileTemplate, const QUrl& baseUrl,
                          KDevelop::TemplateRenderer* renderer);

    /**
     * Returns the file URLs, as specified by the user.
     */
    QHash<QString, QUrl> fileUrls() const;
    /**
     * Returns the positions within files where code is to be generated.
     */
    QHash<QString, KTextEditor::Cursor> filePositions() const;

    void setFocusToFirstEditWidget() override;

Q_SIGNALS:
    /**
     * @copydoc ClassIdentifierPage::isValid
     */
    void isValid(bool valid);

private:
    friend struct OutputPagePrivate;
    struct OutputPagePrivate* const d;
};

}

#endif // KDEVPLATFORM_PLUGIN_OUTPUTPAGE_H
