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
#include <QCommandLineParser>
#include <QFileInfo>
#include <QNetworkProxy>

#include "./network.h"
#include "./sandbox.h"
#include "./stdio.h"
#include "./util.h"


int main(int argc, char * argv[]) {
    /* Prepend "-platform minimal" to the list of arguments. I've given
     * up on trying to find a way to set this programatically. */
    int argc_ = argc + 2;
    char ** argv_ = new char *[argc_];

    argv_[0] = argv[0];
    argv_[1] = (char *) "-platform";
    argv_[2] = (char *) "minimal";

    for (int i = 1; i < argc; i++) {
        argv_[i+2] = argv[i];
    }

    /* Set up our application. */
    QApplication app(argc_, argv_);

    app.setApplicationName("koala");
    app.setApplicationVersion("0.0.1");

    /* Parse command-line options. */
    QCommandLineParser parser;
    QCommandLineOption proxyOption(QStringList() << "p" << "proxy", "Optional HTTP/HTTPS proxy.", "address");

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(proxyOption);

    parser.addPositionalArgument("script", "The .js-file to be executed.");
    parser.process(app);

    QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        parser.showHelp(-1);
        return -1;
    }

    /* Read the main script file. */
    QFileInfo info(args.at(0));
    QString path = info.absoluteFilePath();

    QByteArray buf;
    QString err = readFileUtf8(path, buf);
    if (!err.isNull()) {
        fprintf(stderr, "Couldn't read %s: %s\n", qPrintable(args.at(0)), qPrintable(err));
        return -1;
    }

    /* Initialize our singletons, and hook their signals up. */
    Sandbox * sandbox = new Sandbox(&app);
    StdioHelper * stdio = new StdioHelper(&app);
    NetworkManager * network = new NetworkManager(&app);

    sandbox->setNetworkAccessManager(network);

    QObject::connect(stdio, SIGNAL(received(QString)),
                     sandbox, SIGNAL(messageReceived(QString)));
    QObject::connect(sandbox, SIGNAL(messageSent(QString)),
                     stdio, SLOT(send(QString)));

    /* Did the user request we use a network proxy? */
    if (parser.isSet(proxyOption)) {
        QUrl url = QUrl::fromUserInput(parser.value(proxyOption));
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, url.host(), url.port(8080));
        QNetworkProxy::setApplicationProxy(proxy);
    }

    /* Finally, launch the sandbox environment. */
    sandbox->launch(path, QString::fromUtf8(buf));

    return app.exec();
}
