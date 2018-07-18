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

#ifndef CLIPVIEWER_H
#define CLIPVIEWER_H

#include <QMainWindow>

#define FLAG_LOG_VIEWER     0x01
#define FLAG_TAIL_VIEWER    0x02

class QAction;
class QCheckBox;
class QKeyEvent;
class QLineEdit;
class QString; 
class QTextBrowser;
class QToolBar;
class QIODevice;

class ClipViewer : public QMainWindow
{
    Q_OBJECT
public:
    ClipViewer(const QString& home, int flags, 
                    const QString &title, QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *e);    

private slots:
    void find(bool rev, bool restart);
    void findStd();
    void findRev();
    void enableFind(const QString &text);

    void setPlainSource(const QString &fname);
    void appendContent();

    void enableFindBar();
    void disableFindBar();

private:
    int flags;
    bool htmlOn;

    QTextBrowser *browser;
    QToolBar *topBar;
    QAction *backwardAction;
    QAction *forwardAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *findShowAction;
    QAction *quitAction;

    QToolBar *findBar;
    QLineEdit *findEdt;
    QAction *findAction;
    QAction *findRevAction;
    QCheckBox *caseChk;
    QCheckBox *wordsChk;
    QAction *findCloseAction;
    QString lastSearched;

    QIODevice *source;
};

#endif // CLIPVIEWER_H

// vi:sw=4:ts=4:et:co=80:
