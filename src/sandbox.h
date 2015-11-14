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
#include <QVariant>
#include <QWebElement>
#include <QWebPage>


/* The Sandbox class hosts and manages a JavaScript execution environment. */
class Sandbox : public QWebPage {
    Q_OBJECT

private:
    /* Path and source of the main script file. */
    QString mainPath;
    QString mainSource;

    /* Stores whether or not we've seen the main frame's initial navigation
     * request - to "qrc:/top.html". It is used to block all subsequent
     * navigation requests on the main frame. */
    bool sawFirstNavigation;

    /* Return value set by the JavaScript runtime in response to the last
     * callback request. */
    QVariant callbackValue;

public:
    /* Construct a new Sandbox instance. */
    Sandbox(QObject * parent = NULL)
          : QWebPage(parent)
          , mainPath(QString())
          , mainSource(QString())
          , sawFirstNavigation(false)
          , callbackValue(QVariant()) {
    }

    /* Launch the sandbox environment. This effectively means asking the
     * QWebPage to navigate to "qrc:/top.html". */
    void launch(QString path, QString code);

protected:
    /* Determine whether or not to allow an attempt to navigate
     * to a new page. */
    bool acceptNavigationRequest(QWebFrame * frame,
                                 const QNetworkRequest & request,
                                 QWebPage::NavigationType type);

    /* Handle a message being written to the JavaScript console. */
    void javaScriptConsoleMessage(const QString & message,
                                  int lineNumber, const QString & sourceID);

    /* Handle dialogs requiring user action; `alert`, `confirm` and
     * `prompt` dialogs. */
    void javaScriptAlert(QWebFrame * frame, const QString & message);
    bool javaScriptConfirm(QWebFrame * frame, const QString & message);
    bool javaScriptPrompt(QWebFrame * frame, const QString & message,
                          const QString & defaultValue, QString * result);

    /* Determine whether or not JavaScript execution should be halted.
     *
     * This function is checked by Qt when the JavaScript code has been
     * "running for a long period of time." */
    bool shouldInterruptJavaScript();

signals:
    /* Signal that a new frame has been inserted into the DOM.
     *
     * This is our replacement for the `frameCreated` signal. It is necessary
     * because QWebFrames can't be passed directly to JavaScript through signals
     * without getting converted to empty strings. Instead we have to cast them
     * to QObject, which works just fine for some reason. */
    void frameSpawned(QWebElement document, QObject * frame, QObject * parent);

    /* Signal that the back-end code has requested a callback.
     *
     * The "callback" system is a clunky but straightforward way to get around
     * the fact that the application's C++ side has no way of directly calling
     * a JavaScript function.
     *
     * By listening for events on the `callbackRequested` signal, and setting
     * return values with `setCallbackValue`, the JavaScript runtime can very
     * easily communicate with the C++ side. This exploits the fact that all
     * signal handlers are called before the original `emit` statement
     * finishes - even JavaScript ones. */
    void callbackRequested(QString name, QObject * frame, const QVariantList & args);

    /* Signal that a new message has just been received, or that the user
     * script has just emitted a new message.
     *
     * These signals are managed entirely by JavaScript code running inside
     * the browser. */
    void messageReceived(QString message);
    void messageSent(QString message);

public slots:
    /* Return an object holding the path and source of the main JavaScript
     * file provided by the user. */
    QVariantMap getMainScript();

    /* Read a UTF-8 encoded script file from disk. */
    QVariantMap readScriptFile(QString path);

    /* Set the return value for the last requested callback; used by the
     * JavaScript runtime. See the documentation for `callbackRequested`. */
    void setCallbackValue(const QVariant & value);

    /* Halt execution immediately and exit the process. */
    void exit(int code);

private slots:
    /* Internal handler for the `frameCreated` signal. */
    void onFrameCreated(QWebFrame * frame);

private:
    /* Request a callback and grab the return value in on fell swoop. */
    QVariant requestCallback(QString name, const QWebFrame * frame, const QVariantList & args);
};
