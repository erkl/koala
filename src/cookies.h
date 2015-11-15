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

#include <QNetworkCookie>
#include <QNetworkCookieJar>


/* The `CookieJar` class is our way of attaching some monitoring and
 * overwrite functionality to an underlying `QNetworkCookieJar`. */
class CookieJar : public QNetworkCookieJar {
    Q_OBJECT

public:
    /* Construct a new CookieJar. */
    CookieJar(QObject * parent = NULL)
        : QNetworkCookieJar(parent) {
    }

    /* Remove a cookie from the jar. */
    bool deleteCookie(const QNetworkCookie & cookie);

    /* Add a cookie to the jar. */
    bool insertCookie(const QNetworkCookie & cookie);

    /* Update a cookie already in the jar. */
    bool updateCookie(const QNetworkCookie & cookie);

    /* Overwrite the cookie jar. */
    void setAllCookies(const QList<QNetworkCookie> & cookies);

signals:
    /* Signal used to indicate that the jar's contents have changed. */
    void updated(QList<QNetworkCookie> cookies);
};
