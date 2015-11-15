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

#include <QApplication>
#include <QNetworkRequest>
#include <QWebFrame>
#include <QWebPage>

#include "./cookies.h"
#include "./sandbox.h"
#include "./util.h"


void Sandbox::launch(QString path, QString src) {
    /* Save main script details. */
    this->mainPath = path;
    this->mainSource = src;

    /* This setting is what allows the user script to tinker with the contents
     * of any iframe it creates, regardless of the domain. It's essentially the
     * key to making our iframe-based architecture work. */
    QWebSettings * settings = this->settings();
    settings->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);

    /* Attach our `frameCreated` handler. */
    QObject::connect(this, SIGNAL(frameCreated(QWebFrame *)),
                     this, SLOT(onFrameCreated(QWebFrame *)));

    /* Load the sandbox page, which will serve as the user script's execution
     * environment, and expose the Sandbox instance. */
    QWebFrame * frame = this->mainFrame();
    frame->setUrl(QUrl("qrc:/top.html"));
    frame->addToJavaScriptWindowObject("__bridge", this);
}


bool Sandbox::acceptNavigationRequest(QWebFrame * frame,
                                      const QNetworkRequest & req,
                                      QWebPage::NavigationType type) {
    /* Only allow the very first navigation request on the main frame. */
    if (frame == this->mainFrame()) {
        if (this->sawFirstNavigation)
            return false;

        this->sawFirstNavigation = true;
        return true;
    }

    QVariantList args = QVariantList();
    args += req.url().toString();

    /* Describe the reason behind the navigation request. */
    switch (type) {
    case QWebPage::NavigationTypeLinkClicked:
        args += "link";
        break;
    case QWebPage::NavigationTypeFormSubmitted:
    case QWebPage::NavigationTypeFormResubmitted:
        args += "form";
        break;
    case QWebPage::NavigationTypeReload:
        args += "reload";
        break;
    default:
        args += "other";
    }

    QVariant ret = this->requestCallback("navigate", frame, args);
    if (!ret.isNull() && ret.canConvert<bool>())
        return ret.toBool();

    return true;
}


void Sandbox::javaScriptConsoleMessage(const QString & message, int lineNumber,
                                       const QString & sourceID) {
    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);

    /* TODO: Report this message properly. */
    qDebug() << message;
}


void Sandbox::javaScriptAlert(QWebFrame * frame, const QString & message) {
    if (frame != this->mainFrame())
        this->requestCallback("alert", frame, QVariantList() << message);
}


bool Sandbox::javaScriptConfirm(QWebFrame * frame, const QString & message) {
    if (frame != this->mainFrame()) {
        QVariant ret = this->requestCallback("confirm", frame, QVariantList() << message);
        if (!ret.isNull() && ret.canConvert<bool>())
            return ret.toBool();
    }

    return false;
}


bool Sandbox::javaScriptPrompt(QWebFrame * frame, const QString & message,
                               const QString & defaultValue, QString * result) {
    if (frame != this->mainFrame()) {
        QVariant ret = this->requestCallback("prompt", frame, QVariantList() << message << defaultValue);
        if (!ret.isNull() && ret.canConvert<QString>()) {
            result->append(ret.toString());
            return true;
        }
    }

    return false;
}


bool Sandbox::shouldInterruptJavaScript() {
    /* Interrupting JavaScript execution won't do us any good, because
     * the main user script is running in the same sandbox. */
    return false;
}


QVariantMap Sandbox::getMainScript() {
    QVariantMap out;
    out["path"] = this->mainPath;
    out["src"] = this->mainSource;
    return out;
}


QVariantMap Sandbox::readScriptFile(QString path) {
    QVariantMap out;
    QByteArray buf;

    QString err = readFileUtf8(path, buf);
    if (!err.isNull()) {
        out["error"] = err;
    } else {
        out["path"] = path;
        out["src"] = QString::fromUtf8(buf);
    }

    return out;
}


void Sandbox::setCallbackValue(const QVariant & value) {
    this->callbackValue = value;
}


static QVariantMap cookieToVariant(const QNetworkCookie & cookie) {
    QVariantMap raw;

    /* Encode basic properties. */
    raw["name"] = QString(cookie.name());
    raw["value"] = QString(cookie.value());
    raw["domain"] = cookie.domain();
    raw["path"] = cookie.path();

    /* Encode the expiration date. */
    if (cookie.expirationDate().isValid())
        raw["expires"] = cookie.expirationDate().toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz UTC");
    else
        raw["expires"] = QVariant();

    /* Encode security flags. */
    raw["isHttpOnly"] = cookie.isHttpOnly();
    raw["isSecure"] = cookie.isSecure();

    return raw;
}


static QNetworkCookie cookieFromVariant(const QVariantMap & raw) {
    QNetworkCookie cookie;

    /* Parse core properties. */
    QVariant name = raw["name"];
    QVariant value = raw["value"];
    QVariant domain = raw["domain"];
    QVariant path = raw["path"];

    if (!name.isNull() && name.canConvert<QByteArray>())
        cookie.setName(name.toByteArray());
    if (!value.isNull() && value.canConvert<QByteArray>())
        cookie.setValue(value.toByteArray());
    if (!domain.isNull() && domain.canConvert<QString>())
        cookie.setDomain(domain.toString());
    if (!path.isNull() && path.canConvert<QString>())
        cookie.setPath(path.toString());

    /* Parse the expiration date. */
    QVariant expires = raw["expires"];

    if (!expires.isNull() && expires.canConvert<QString>()) {
        QDateTime time = QDateTime::fromString(expires.toString(), "yyyy-MM-dd hh:mm:ss.zzz UTC");
        if (time.isValid())
            cookie.setExpirationDate(time);
    }

    /* Parse security flags. */
    QVariant isHttpOnly = raw["isHttpOnly"];
    QVariant isSecure = raw["isSecure"];

    if (!isHttpOnly.isNull() && isHttpOnly.canConvert<bool>())
        cookie.setHttpOnly(isHttpOnly.toBool());
    if (!isSecure.isNull() && isSecure.canConvert<bool>())
        cookie.setSecure(isSecure.toBool());

    return cookie;
}


void Sandbox::onCookiesChanged(QList<QNetworkCookie> cookies) {
    QVariantList list;
    foreach (QNetworkCookie cookie, cookies)
        list += cookieToVariant(cookie);

    emit this->cookiesChanged(list);
}


void Sandbox::setCookies(const QVariant & raw) {
    QList<QNetworkCookie> cookies;

    foreach (QVariant item, raw.toList()) {
        cookies += cookieFromVariant(item.toMap());
    }

    CookieJar * jar = (CookieJar *) this->networkAccessManager()->cookieJar();
    jar->setAllCookies(cookies);
}


void Sandbox::exit(int code) {
    QApplication::instance()->exit(code);
}


void Sandbox::onFrameCreated(QWebFrame * frame) {
    QWebFrame * parent = frame->parentFrame();
    if (parent == this->mainFrame())
        parent = NULL;

    emit this->frameSpawned(frame->documentElement(), (QObject *) frame, (QObject *) parent);
}


QVariant Sandbox::requestCallback(QString name, const QWebFrame * frame, const QVariantList & args) {
    emit this->callbackRequested(name, (QObject *) frame, args);
    return this->callbackValue;
}
