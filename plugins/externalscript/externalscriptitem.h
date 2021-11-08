/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H
#define KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H

#include <QStandardItem>

class QAction;

/**
 * NOTE: use @c text() and @c setText() to define the label/name of the external script.
 */
class ExternalScriptItem
    : public QStandardItem
{
public:
    ExternalScriptItem();

    /**
     * The key is supposed to be unique inside the model
     * @return The key of this script item
     */
    QString key() const;
    /**
     * Sets the label
     */
    void setKey(const QString& key);

    /**
     * @return The command to execute.
     */
    QString command() const;
    /**
     * Sets the command to execute.
     */
    void setCommand(const QString& command);

    /**
     * @return The working directory where to execute the command.
     *         If this is empty (default), it should be derived from the active document.
     */
    QString workingDirectory() const;

    /**
     * Specify the working directory where the command should be executed
     */
    void setWorkingDirectory(const QString& workingDirectory);

    /**
     * Whether placeholders like %b etc. in the command should be substituted. Default is true.
     * */
    bool performParameterReplacement() const;

    /**
     * Set whether placeholders like %b etc. in the command should be substituted. Default is true.
     * */
    void setPerformParameterReplacement(bool perform);

    enum SaveMode {
        /// Nothing needs to be saved.
        SaveNone,
        /// Currently active document gets saved.
        SaveCurrentDocument,
        /// All opened documents get saved.
        SaveAllDocuments
    };
    /**
     * @return @c SaveMode that decides what document should be saved before executing this script.
     */
    SaveMode saveMode() const;
    /**
     * Sets the @c SaveMode that decides what document should be saved before executing this script.
     */
    void setSaveMode(SaveMode mode);

    /**
     * @return what type of filter should be applied to the execution of the external script
     **/
    int filterMode() const;

    /**
     * Sets the filtering mode
     **/
    void setFilterMode(int mode);

    /// Defines what should be done with the @c STDOUT of a script run.
    enum OutputMode {
        /// Ignore output and do nothing.
        OutputNone,
        /// Output gets inserted at the cursor position of the current document.
        OutputInsertAtCursor,
        /// Current selection gets replaced in the active document.
        /// If no selection exists, the output will get inserted at the
        /// current cursor position in the active document view.
        OutputReplaceSelectionOrInsertAtCursor,
        /// Current selection gets replaced in the active document.
        /// If no selection exists, the whole document gets replaced.
        OutputReplaceSelectionOrDocument,
        /// The whole contents of the active document gets replaced.
        OutputReplaceDocument,
        /// Create a new file from the output.
        OutputCreateNewFile
    };
    /**
     * @return @c OutputMode that decides what parts of the active document should be replaced by the
     *         @c STDOUT of the @c command() execution.
     */
    OutputMode outputMode() const;
    /**
     * Sets the @c OutputMode that decides what parts of the active document should be replaced by the
     * @c STDOUT of the @c command() execution.
     */
    void setOutputMode(OutputMode mode);

    /// Defines what should be done with the @c STDERR of a script run.
    enum ErrorMode {
        /// Ignore errors and do nothing.
        ErrorNone,
        /// Merge with @c STDOUT and use @c OutputMode.
        ErrorMergeOutput,
        /// Errors get inserted at the cursor position of the current document.
        ErrorInsertAtCursor,
        /// Current selection gets replaced in the active document.
        /// If no selection exists, the output will get inserted at the
        /// current cursor position in the active document view.
        ErrorReplaceSelectionOrInsertAtCursor,
        /// Current selection gets replaced in the active document.
        /// If no selection exists, the whole document gets replaced.
        ErrorReplaceSelectionOrDocument,
        /// The whole contents of the active document gets replaced.
        ErrorReplaceDocument,
        /// Create a new file from the errors.
        ErrorCreateNewFile
    };

    /**
     * @return @c ErrorMode that decides what parts of the active document should be replaced by the
     *         @c STDERR of the @c command() execution.
     */
    ErrorMode errorMode() const;
    /**
     * Sets the @c ErrorMode that decides what parts of the active document should be replaced by the
     * @c STDERR of the @c command() execution.
     */
    void setErrorMode(ErrorMode mode);

    enum InputMode {
        /// Nothing gets streamed to the @c STDIN of the external script.
        InputNone,
        /// Current selection gets streamed into the @c STDIN of the external script.
        /// If no selection exists, nothing gets streamed.
        InputSelectionOrNone,
        /// Current selection gets streamed into the @c STDIN of the external script.
        /// If no selection exists, the whole document gets streamed.
        InputSelectionOrDocument,
        /// The whole contents of the active document get streamed into the @c STDIN of the external script.
        InputDocument,
    };
    /**
     * @return @c InputMode that decides what parts of the active document should be streamded into
     *         the @c STDIN of the external script.
     */
    InputMode inputMode() const;
    /**
     * Sets the @c InputMode that decides what parts of the active document should be streamded into
     * the @c STDIN of the external script.
     */
    void setInputMode(InputMode mode);

    /**
     * Action to trigger insertion of this snippet.
     */
    QAction* action();

    /**
     * @return True when this command should have its output shown, false otherwise.
     */
    bool showOutput() const;
    /**
     * Set @p show to true when the output of this command shout be shown, false otherwise.
     */
    void setShowOutput(bool show);

    ///TODO: custom icon
    ///TODO: mimetype / language filter
    ///TODO: kate commandline integration
    ///TODO: filter for local/remote files

    /**
     * Saves this item after changes.
     */
    void save() const;

private:
    QString m_key;
    QString m_command;
    QString m_workingDirectory;
    SaveMode m_saveMode = SaveNone;
    OutputMode m_outputMode = OutputNone;
    ErrorMode m_errorMode = ErrorNone;
    InputMode m_inputMode = InputNone;
    QAction* m_action = nullptr;
    bool m_showOutput = true;
    int m_filterMode = 0;
    bool m_performReplacements = true;
};

Q_DECLARE_METATYPE(ExternalScriptItem*)

#endif // KDEVPLATFORM_PLUGIN_EXTERNALSCRIPTITEM_H
