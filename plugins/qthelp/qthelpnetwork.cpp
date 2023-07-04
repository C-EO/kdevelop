/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpnetwork.h"

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngineCore* engine, QObject* parent)
    : QNetworkAccessManager(parent), m_helpEngine(engine)
{}

HelpNetworkAccessManager::~HelpNetworkAccessManager()
{
}

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData, const QString &mimeType)
    : data(fileData), origLen(fileData.length())
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);

    // Instantly finish processing if data is empty. Without this code the loadFinished()
    // signal will never be emitted by the corresponding QWebView.
    if (!origLen) {
        qCDebug(QTHELP) << "Empty data for" << request.url().toDisplayString();
        QTimer::singleShot(0, this, &QNetworkReply::finished);
    }

#ifdef USE_QTWEBKIT
    // Fix broken CSS images (tested on Qt 5.5.1, 5.7.0, 5.9.7 and 5.14.1)
    if (request.url().fileName() == QLatin1String("offline.css")) {
        data.replace("../images", "images");
    }
#endif

    // Fix flickering when loading, the page has the offline-simple.css stylesheet which is replaced
    // later by offline.css  by javascript which causes flickering so we force the full stylesheet
    // from the beginning
    if (request.url().fileName().endsWith(QLatin1String(".html"))) {
        data.replace("offline-simple.css", "offline.css");
    }

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
    QTimer::singleShot(0, this, &QNetworkReply::metaDataChanged);
    QTimer::singleShot(0, this, &QIODevice::readyRead);
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
	qint64 len = qMin(qint64(data.length()), maxlen);
	if (len) {
		memcpy(buffer, data.constData(), len);
		data.remove(0, len);
	}
	if (!data.length())
		QTimer::singleShot(0, this, &QNetworkReply::finished);
	return len;
}

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
	QString scheme = request.url().scheme();
	if (scheme == QLatin1String("qthelp") || scheme == QLatin1String("about")) {
		QString mimeType = QMimeDatabase().mimeTypeForUrl(request.url()).name();
		if (mimeType == QLatin1String("application/x-extension-html")) {
			// see also: https://bugs.kde.org/show_bug.cgi?id=288277
			// firefox seems to add this bullshit mimetype above
			// which breaks displaying of qthelp documentation :(
			mimeType = QStringLiteral("text/html");
		}
		return new HelpNetworkReply(request, m_helpEngine->fileData(request.url()), mimeType);
	}
	return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

#include "moc_qthelpnetwork.cpp"
