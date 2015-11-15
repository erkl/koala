/* Copyright (c) 2015, Erik Lundin.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. */

#include <QNetworkReply>
#include <QNetworkRequest>

#include "./network.h"


QNetworkReply * NetworkManager::createRequest(QNetworkAccessManager::Operation op,
                                              const QNetworkRequest & req,
                                              QIODevice * data) {
    /* Prevent requests with the "qrc" and "file" schemes (except the very
     * first request, which is necessary for loading "qrc://top.html"). */
    QString scheme = req.url().scheme().toLower();
    if (scheme == "qrc") {
        if (this->sawFirstQRCRequest == false)
            this->sawFirstQRCRequest = true;
        else
            return new BlockedReply(op, req);
    } else if (scheme == "file") {
        return new BlockedReply(op, req);
    }

    /* Fall back to the default implementation. */
    return QNetworkAccessManager::createRequest(op, req, data);
}


BlockedReply::BlockedReply(QNetworkAccessManager::Operation op,
                           const QNetworkRequest & req,
                           QObject * parent)
                         : QNetworkReply(parent) {
    /* This constructor function has been lifted, pretty much verbatim,
     * from Qt's internal `QDisabledNetworkReply`. Seems to work. */
    this->setRequest(req);
    this->setUrl(req.url());
    this->setOperation(op);

    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    this->setError(UnknownNetworkError, "Request blocked by koala.");

    QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection, Q_ARG(QNetworkReply::NetworkError, UnknownNetworkError));
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}
