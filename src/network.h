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

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "./sandbox.h"


/* The NetworkManager class implements our custom logic for dealing with
 * network requests for resources. */
class NetworkManager : public QNetworkAccessManager {
    Q_OBJECT

private:
    /* Stores whether we've seen the first request for a QRC resource;
     * specifically "qrc://top.html". */
    bool sawFirstQRCRequest;

public:
    /* Constructs a new NetworkManager instance. */
    NetworkManager(QObject * parent = NULL)
                 : QNetworkAccessManager(parent)
                 , sawFirstQRCRequest(false) {
    }

protected:
    /* Creates a QNetworkReply in response to the request. */
    QNetworkReply * createRequest(QNetworkAccessManager::Operation op,
                                  const QNetworkRequest & req,
                                  QIODevice * data = NULL);

signals:
    /* Signal emitted when a request is blocked due to its URL scheme. */
    void requestBlocked(QObject * origin, QUrl url);
};


/* Implementation of a blocked network reply, based almost entirely on
 * Qt's internal `QDisabledNetworkReply`. */
class BlockedReply : public QNetworkReply {
    Q_OBJECT

public:
    /* Constructs a blocked reply. */
    BlockedReply(QNetworkAccessManager::Operation op,
                 const QNetworkRequest & req,
                 QObject * parent = NULL);

    /* Cancels the request. */
    void abort() { }

protected:
    /* Reads more incoming data. */
    qint64 readData(char *, qint64) { return -1; }
};
