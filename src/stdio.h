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

#include <QByteArray>
#include <QSocketNotifier>


/* The StdioHelper class reads/writes lines of UTF-8 text to stdin/stdout. */
class StdioHelper : public QObject {
    Q_OBJECT

private:
    /* Socket notifier attached to stdin. */
    QSocketNotifier * notifier;

    /* Buffer storing input data until a complete line has been read. */
    QByteArray buffer;

public:
    /* Constructor. */
    StdioHelper(QObject * parent = NULL);

public slots:
    /* Write a message as a single line to stdout. */
    void send(QString message);

signals:
    /* Signal emitted whenever a line of input has been read from stdin. */
    void received(QString message);

private slots:
    /* This function handles the signals emitted by our QSocketNotifier
     * telling is that there is more data to be read from stdin. */
    void onReadReady();
};
