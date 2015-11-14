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

#include <sys/select.h>

#include "./stdio.h"


StdioHelper::StdioHelper(QObject * parent)
    : QObject(parent)
    , notifier(new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this)) {
    QObject::connect(this->notifier, SIGNAL(activated(int)),
                     this, SLOT(onReadReady()));
}


void StdioHelper::send(QString message) {
    fprintf(stdout, "%s\n", qPrintable(message));
}


void StdioHelper::onReadReady() {
    struct timeval tv = {0, 0};
    fd_set fds;
    char buf[1024];

    FD_ZERO(&fds);

    /* Keep reading from stdin until we're out of data. */
    for (;;) {
        FD_SET(STDIN_FILENO, &fds);

        /* Use `select` with a zero timeout to poll for data. */
        if (select(1, &fds, NULL, NULL, &tv) <= 0)
            break;

        ssize_t rem = read(STDIN_FILENO, buf, 1024);
        if (rem <= 0)
            break;

        char * cur = buf;

        for (;;) {
            /* Look for the next line feed character. */
            ssize_t newline = -1;

            for (ssize_t i = 0; i < rem; i++) {
                if (cur[i] == '\n') {
                    newline = i + 1;
                    break;
                }
            }

            /* With no newline in sight, store what we have and wait for
             * another chunk of data. */
            if (newline < 0) {
                this->buffer.append(cur, rem);
                break;
            }

            /* Let any listeners know we've now read a full line. */
            this->buffer.append(cur, newline - 1);
            emit this->received(QString::fromUtf8(this->buffer));
            this->buffer.truncate(0);

            /* Move forward. */
            cur += newline;
            rem -= newline;
        }
    }
}
