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
#include <QAction>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextDocument>
#include <QToolBar>
#include <QDesktopWidget>

ClipViewer::ClipViewer(const QString &infile, int flags, const QString &title, 
                            QWidget* parent)
    : QMainWindow(parent, Qt::Window ), flags(flags), htmlOn(true)
{
    browser = new QTextBrowser(this);

    browser->setFrameStyle(QFrame::Panel|QFrame::Sunken );
    browser->setAlignment(Qt::AlignJustify);
    browser->setWordWrapMode(QTextOption::WordWrap);
    if (flags & FLAG_LOG_VIEWER) {
        browser->setAcceptRichText(false);
        htmlOn = false;
    }
    browser->setOpenExternalLinks(false);

    setCentralWidget(browser);

    backwardAction = new QAction(QIcon(":/icons/backward.png"), 
                                                    "Précédent", this);
    connect(backwardAction, SIGNAL(triggered()), browser, SLOT(backward()));
    connect(browser, SIGNAL(backwardAvailable(bool)), 
                                    backwardAction, SLOT(setEnabled(bool)));
    backwardAction->setShortcut(Qt::CTRL|Qt::Key_Left);

    forwardAction = new QAction(QIcon(":/icons/forward.png"), "Suivant", this);
    connect(forwardAction, SIGNAL(triggered()), browser, SLOT(forward()));
    connect(browser, SIGNAL(forwardAvailable(bool)), 
                                    forwardAction, SLOT(setEnabled(bool)));
    forwardAction->setShortcut(Qt::CTRL|Qt::Key_Right);

    zoomOutAction = new QAction(QIcon(":/icons/zoom-out.png"), 
                                            "Zoom arrière", this);
    connect(zoomOutAction, SIGNAL(triggered()), browser, SLOT(zoomOut()));
    zoomOutAction->setShortcut(Qt::CTRL|Qt::Key_Minus);

    zoomInAction = new QAction(QIcon(":/icons/zoom-in.png"), 
                                            "Zoom avant", this);
    connect(zoomInAction, SIGNAL(triggered()), browser, SLOT(zoomIn()));
    zoomInAction->setShortcut(Qt::CTRL|Qt::Key_Plus);

    findShowAction = new QAction(QIcon(":/icons/find.png"), "Rechercher", this);
    connect(findShowAction, SIGNAL(triggered()), this, SLOT(enableFindBar()));
    findShowAction->setShortcut(Qt::CTRL|Qt::Key_F);

    quitAction = new QAction(QIcon(":/icons/exit.png"), "Fermer", this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    quitAction->setShortcut(Qt::CTRL|Qt::Key_Q);

    topBar = new QToolBar(this);
    topBar->setMovable(false);
    topBar->setFloatable(false);
    topBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    topBar->addAction(backwardAction);
    topBar->addAction(forwardAction);
    topBar->addSeparator();
    topBar->addAction(zoomOutAction);
    topBar->addAction(zoomInAction);
    topBar->addSeparator();
    topBar->addAction(findShowAction);
    topBar->addSeparator();
    topBar->addAction(quitAction);
    addToolBar(Qt::TopToolBarArea, topBar);

    findEdt = new QLineEdit(this);
    connect(findEdt, SIGNAL(textChanged(const QString&)),
                        this, SLOT(enableFind(const QString&)));

    findRevAction = new QAction(QIcon(":/icons/backward.png"), 
                                                        "Précédent", this);
    connect(findRevAction, SIGNAL(triggered()), this, SLOT(findRev()));
    findRevAction->setEnabled(false);

    findAction = new QAction(QIcon(":/icons/forward.png"), "Suivant", this);
    connect(findAction, SIGNAL(triggered()), this, SLOT(findStd()));
    findAction->setEnabled(false);

    caseChk = new QCheckBox("Respecter la casse", this);
    wordsChk = new QCheckBox("Mots entiers", this);

    findCloseAction = new QAction(QIcon(":/icons/close.png"), "", this);
    connect(findCloseAction, SIGNAL(triggered()), 
                                this, SLOT(disableFindBar()));
    findCloseAction->setShortcut(Qt::Key_Escape);

    lastSearched = "";

    findBar = new QToolBar(this);
    findBar->setMovable(false);
    findBar->setFloatable(false);
    findBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    findBar->addWidget(new QLabel("Rechercher :", 0));
    findBar->addWidget(findEdt);
    findBar->addAction(findRevAction);
    findBar->addAction(findAction);
    findBar->addWidget(caseChk);
    findBar->addWidget(wordsChk);
    findBar->addSeparator();
    findBar->addAction(findCloseAction);
    addToolBar(Qt::BottomToolBarArea, findBar);
    findBar->setVisible(false);

    source = NULL;
    if (!infile.isEmpty()) {
        if (htmlOn)
            browser->setSource(infile);
        else
            setPlainSource(infile);
        if (!title.isEmpty())
            setWindowTitle(title);
        else
            setWindowTitle(browser->documentTitle());
    }

    QRect screen = QApplication::desktop()->screenGeometry();
    setMinimumSize(780, screen.height() - 180);
    setMaximumSize(780, screen.height() - 150);

    browser->setFocus();
}

void 
ClipViewer::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
        /*
        case Qt::Key_Escape:
            e->accept();
            qApp->quit();
            break;
        */
        case Qt::Key_Return:
            e->accept();
            find(false, false);
            break;
    }
}

void
ClipViewer::find(bool rev, bool restart)
{
    QString search = findEdt->text();
    QTextDocument *doc = browser->document();
    bool found = false;
    QTextDocument::FindFlags flags = 0;
    if (rev)
        flags |= QTextDocument::FindBackward;
    if (caseChk->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (wordsChk->isChecked())
        flags |= QTextDocument::FindWholeWords;
        
   
    if (search == "")
        return;

    QTextCursor searchCursor = browser->textCursor();

    if (restart) {
        if (rev)
            searchCursor.movePosition(QTextCursor::End);
        else 
            searchCursor.movePosition(QTextCursor::Start);
    } else if (lastSearched == search) {
        if (rev) {
            searchCursor.movePosition(QTextCursor::WordLeft);
            searchCursor.movePosition(QTextCursor::WordLeft);
        } else
            searchCursor.movePosition(QTextCursor::WordRight);
    }

    searchCursor = doc->find(search, searchCursor, flags);

    if (searchCursor.isNull()) {
        int yesno;
        yesno = QMessageBox::question(0, "Motif non trouvé", 
                QString("Le motif recherché n'a pas été trouvé.\n"
                        "Continuer la recherche depuis %1 du document ?")
                       .arg((rev) ? "la fin" : "le début"),
                QMessageBox::Yes, QMessageBox::No);
        if (yesno == QMessageBox::Yes)
            return find(rev, true);
        else
            return;
    } else {
        found = true;
        searchCursor.movePosition(QTextCursor::WordRight,
                               QTextCursor::KeepAnchor);

        browser->setTextCursor(searchCursor);
    }

    lastSearched = search;
}

void
ClipViewer::findStd()
{
    find(false, false);
}

void
ClipViewer::findRev()
{
    find(true, false);
}

void
ClipViewer::enableFind(const QString &text)
{
    bool on = !text.isEmpty();
    findAction->setEnabled(on);
    findRevAction->setEnabled(on);
}

void
ClipViewer::setPlainSource(const QString &fname)
{
    if (source) {
        source->close();
        delete source;
        source = NULL;
    }

    QFile in(fname);
    if (!in.open(QIODevice::ReadOnly)) 
        return;
    QString txt = in.readAll();
    if (txt.endsWith("\n"))
        txt.chop(1);
    browser->setPlainText(txt);
    in.close();

    if (flags & FLAG_TAIL_VIEWER) {
        QTextCursor cur = browser->textCursor();
        cur.movePosition(QTextCursor::End);
        browser->setTextCursor(cur);

        QProcess *tail = new QProcess(this);
        QStringList args;
        args << "-n" << "0" << "-f" << fname;
        tail->start("tail", args, QIODevice::ReadOnly);
        connect(tail, SIGNAL(readyReadStandardOutput()), 
                                    this, SLOT(appendContent()));
        source = tail;
    }
    backwardAction->setEnabled(false);
    forwardAction->setEnabled(false);
}

void
ClipViewer::appendContent()
{
    if (!source)
        return;

    QTextCursor cur = browser->textCursor();
    bool move = cur.atEnd();
    
    QString txt = source->readAll();
    if (txt.endsWith("\n"))
        txt.chop(1);
    browser->append(txt);
    if (move) {
        cur.movePosition(QTextCursor::End);
        browser->setTextCursor(cur);
    }
}

void
ClipViewer::enableFindBar()
{
    if (findBar && !findBar->isVisible()) {
        findBar->setVisible(true);
        findShowAction->setEnabled(false);
        findEdt->setFocus();
    }
}

void
ClipViewer::disableFindBar()
{
    if (findBar && findBar->isVisible()) {
        findBar->setVisible(false);
        findShowAction->setEnabled(true);
        browser->setFocus();
    }
}
// vi:sw=4:ts=4:et:co=80:
