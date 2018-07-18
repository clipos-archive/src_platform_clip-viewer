// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2010-2018 ANSSI. All Rights Reserved.
/*
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * Author: Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Largely based on the "helpviewer" example from the Qt3 toolkit
 * distribution, which is Copyright (C) 1992-2008 Trolltech ASA. 
 * Distributed under the terms of the GNU General Public License,
 * version 2.
 */

#include "clipviewer.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QTextCodec>
#include <QTranslator>
#include <stdlib.h>
#include <unistd.h>

int 
main(int argc, char **argv)
{
    char c;
    int flags = 0;

    while ((c = getopt(argc, argv, "lt")) != -1) {
        switch (c) {
            case 'l':
                flags |= FLAG_LOG_VIEWER;
                break;
            case 't':
                flags |= FLAG_TAIL_VIEWER;
                break;
            default:
                qDebug("Unknown option : %c", c);
                break;
        }
    }
    argc -= optind;
    argv += optind;

    QApplication a(argc, argv);
    QTranslator translator(0);
    translator.load("qt_fr.qm", PREFIX"/share/qt4/translations/");
    a.installTranslator( &translator );

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForCStrings(codec);

    QString home;
    if (argc > 0) {
        home = argv[0];
    } else {
    	return EXIT_FAILURE;
    }
    QString title;
    if (argc > 1) {
        title = argv[1];
    }

    ClipViewer *help = new ClipViewer(home, flags, title);
    if (QApplication::desktop()->width() > 700
	        && QApplication::desktop()->height() > 600) {
	    help->show();
    } else {
	    help->showMaximized();
    }

    return a.exec();
}

// vi:sw=4:ts=4:et:co=80:

