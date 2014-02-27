/****************************************************************************
**                                                                         **
** Copyright (C) 2009 Victor Zinkevich. All rights reserved.               **
** Contact: vicking@yandex.ru                                              **
**                                                                         **
** This file is part of the Algorithm Flowchart Editor project.            **
**                                                                         **
** This file may be used under the terms of the GNU                        **
** General Public License versions 2.0 or 3.0 as published by the Free     **
** Software Foundation and appearing in the file LICENSE.TXT included in   **
** the packaging of this file.                                             **
** You can find license at http://www.gnu.org/licenses/gpl.html            **
**                                                                         **
****************************************************************************/

#include "thelpwindow.h"



THelpWindow::THelpWindow()
{
  fWidget = new QFrame(this);
  setWidget(fWidget);
  toolBar = new QToolBar;
  textBrowser = new QTextBrowser;
  QVBoxLayout *vl = new QVBoxLayout;
  vl->addWidget(toolBar);
  vl->addWidget(textBrowser);
  widget()->setLayout(vl);
  textBrowser->setSearchPaths(QStringList() << "./doc");
#ifdef Q_WS_X11
  textBrowser->setSearchPaths(QStringList() << "/usr/share/doc/packages/afce");
#endif

  home();
  toolBar->addAction(QIcon(":/images/back_16_h.png"), tr("Back"), textBrowser, SLOT(backward()));
  toolBar->addAction(QIcon(":/images/forward_16_h.png"), tr("Forward"), textBrowser, SLOT(forward()));
  toolBar->addSeparator();
  toolBar->addAction(QIcon(":/images/home_16_h.png"), tr("Home"), this, SLOT(home()));
}

void THelpWindow::home()
{
  textBrowser->setSource(QUrl("index.html"));
}
