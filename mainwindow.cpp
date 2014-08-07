/****************************************************************************
**                                                                         **
** Copyright (C) 2009-2014 Victor Zinkevich. All rights reserved.          **
** Contact: vicking@yandex.ru                                              **
**                                                                         **
** This file is part of the Algorithm Flowchart Editor project.            **
**                                                                         **
** This file may be used under the terms of the GNU                        **
** General Public License versions 2.0 or 3.0 as published by the Free     **
** Software Foundation and appearing in the file LICENSE included in       **
** the packaging of this file.                                             **
** You can find license at http://www.gnu.org/licenses/gpl.html            **
**                                                                         **
****************************************************************************/

#include "mainwindow.h"
#include "zvflowchart.h"
#include <QtGui>
#include <QtSvg>
#include <QSettings>
#include "qflowchartstyle.h"
#include "zvcodegen.h"
#if QT_VERSION >= 0x050000
    #include <QtPrintSupport/QPrinter>
    #include <QtPrintSupport/QPrintDialog>
#endif

QString afceVersion()
{
    return "0.9.6";
}


void AfcScrollArea::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    emit mouseDown();
}


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), fDocument(0)
{
    setupUi();
    readSettings();
    writeSettings();
    retranslateUi();

    QFlowChart *fc = new QFlowChart(this);
    setDocument(fc);
    document()->setZoom(1);
    connect(document(), SIGNAL(statusChanged()), this, SLOT(slotStatusChanged()));
    connect(document(), SIGNAL(editBlock(QBlock *)), this, SLOT(slotEditBlock(QBlock *)));
    connect(actUndo, SIGNAL(triggered()), document(), SLOT(undo()));
    connect(actRedo, SIGNAL(triggered()), document(), SLOT(redo()));
    connect(document(), SIGNAL(changed()), this, SLOT(updateActions()));
    connect(document(), SIGNAL(changed()), this, SLOT(generateCode()));
    document()->setStatus(QFlowChart::Selectable);
    connect(saScheme, SIGNAL(mouseDown()), document(), SLOT(deselectAll()));
    connect(codeLanguage, SIGNAL(activated(int)), this, SLOT(codeLangChanged(int)));

    QFlowChartStyle st;
    QPalette pal = palette();
    st.setLineWidth(2);
    st.setNormalBackground(pal.color(QPalette::Base));
    st.setNormalForeground(pal.color(QPalette::WindowText));
    st.setNormalMarker(Qt::red);
    st.setSelectedBackground(pal.color(QPalette::Highlight));
    st.setSelectedForeground(pal.color(QPalette::HighlightedText));
    st.setNormalMarker(Qt::green);
    st.setFontSize(10);
    document()->setChartStyle(st);

    labelMenu = new QLabel(statusBar());

    statusBar()->setSizeGripEnabled(false);
    statusBar()->addWidget(labelMenu);
    labelMenu->setAlignment(Qt::AlignCenter);

}


void MainWindow::writeSettings()
{
    QSettings settings("Moose Soft", "afce");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());


    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::readSettings()

{
    QSettings settings("Moose Soft", "afce");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(900, 500)).toSize());
    move(settings.value("pos", QPoint(200, 100)).toPoint());
    settings.endGroup();

}
void MainWindow::setupUi()
{
    QApplication::setWindowIcon(QIcon(":/images/icon.png"));
    createActions();
    createMenu();
    createToolBar();
    QWidget *body = new QWidget;
    QWidget *zoomPanel = new QWidget;
    zoomPanel->setMinimumHeight(18);
    saScheme = new AfcScrollArea();
    QPalette pal = saScheme->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Base));
    saScheme->setPalette(pal);

    setCentralWidget(body);
    QVBoxLayout *bodyLayout = new QVBoxLayout;
    bodyLayout->addWidget(saScheme);
    bodyLayout->addWidget(zoomPanel);
    body->setLayout(bodyLayout);
    QSlider *zoomSlider = new QSlider(Qt::Horizontal, zoomPanel);
    zoomLabel = new QLabel;
    zl = new QLabel;
    QHBoxLayout *zoomLayout= new QHBoxLayout;
    zoomLayout->addStretch();
    zoomLayout->addWidget(zl);
    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(zoomSlider);
    zoomPanel->setLayout(zoomLayout);
    zoomSlider->setRange(10, 500);
    zoomSlider->setSingleStep(10);
    zoomSlider->setPageStep(100);
    connect(zoomSlider, SIGNAL(valueChanged(int)),this, SLOT(setZoom(int)));
    zoomSlider->setValue(100);
    createToolbox();

    dockCode = new QDockWidget(this);
    dockCode->setObjectName("dock_code");
    dockCode->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, dockCode);
    codeWidget = new QFrame;
    codeLanguage = new QComboBox;
    codeText = new QTextEdit;
    codeLabel = new QLabel;
    connect(dockCode, SIGNAL(visibilityChanged(bool)), this, SLOT(docCodeVisibilityChanged(bool)));

    codeWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

    codeLabel->setBuddy(codeLanguage);

    codeText->setFont(QFont("Courier New", 12));
    codeText->setLineWrapMode(QTextEdit::NoWrap);
    codeText->setReadOnly(true);


    QVBoxLayout * vbl = new QVBoxLayout;
    vbl->addWidget(codeLabel);
    vbl->addWidget(codeLanguage);
    vbl->addWidget(codeText);
    codeWidget->setLayout(vbl);
    dockCode->setWidget(codeWidget);

    helpWindow = new THelpWindow();
    connect(helpWindow, SIGNAL(windowVisibilityChanged()), this, SLOT(helpWindowHidden()));
    helpWindow->setObjectName("help_window");
    helpWindow->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::RightDockWidgetArea, helpWindow);
    helpWindow->hide();

}

QToolButton * createToolButton(const QString & fileName)
{
    QToolButton *Result = new QToolButton;
    Result->setIconSize(QSize(32, 32));
    Result->setIcon(QIcon(fileName));
    Result->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    Result->setAutoRaise(true);
    Result->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    Result->setCheckable(true);
    Result->setAutoExclusive(true);
    return Result;
}

void MainWindow::createToolbox()
{
    dockTools = new QDockWidget(this);
    dockTools->setObjectName("dock_tools");
    dockTools->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockTools->setMinimumWidth(150);
    addDockWidget(Qt::LeftDockWidgetArea, dockTools);
    connect(dockTools, SIGNAL(visibilityChanged(bool)), this, SLOT(docToolsVisibilityChanged(bool)));

    tbArrow = createToolButton(":/images/arrow.png");
    tbArrow->setChecked(true);
    tbProcess = createToolButton(":/images/simple.png");
    tbIf = createToolButton(":/images/if.png");
    tbFor = createToolButton(":/images/for.png");
    tbWhilePre = createToolButton(":/images/while.png");
    tbWhilePost = createToolButton(":/images/until.png");
    tbIo = createToolButton(":/images/io.png");
    tbOu = createToolButton(":/images/ou.png");
    tbForCStyle = createToolButton(":/images/forc.png");

    connect(tbArrow, SIGNAL(pressed()), this, SLOT(slotToolArrow()));
    connect(tbProcess, SIGNAL(pressed()), this, SLOT(slotToolProcess()));
    connect(tbIf, SIGNAL(pressed()), this, SLOT(slotToolIf()));
    connect(tbFor, SIGNAL(pressed()), this, SLOT(slotToolFor()));
    connect(tbWhilePre, SIGNAL(pressed()), this, SLOT(slotToolWhilePre()));
    connect(tbWhilePost, SIGNAL(pressed()), this, SLOT(slotToolWhilePost()));
    connect(tbIo, SIGNAL(pressed()), this, SLOT(slotToolIo()));
    connect(tbOu, SIGNAL(pressed()), this, SLOT(slotToolOu()));
    connect(tbForCStyle, SIGNAL(pressed()), this, SLOT(slotToolForCStyle()));

    toolsWidget = new QFrame;
    toolsWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QVBoxLayout *tl = new QVBoxLayout;
    tl->setSpacing(2);
    tl->addWidget(tbArrow);
    tl->addWidget(tbIo);
    tl->addWidget(tbOu);
    tl->addWidget(tbProcess);
    tl->addWidget(tbIf);
    tl->addWidget(tbFor);
    tl->addWidget(tbWhilePre);
    tl->addWidget(tbWhilePost);
    tl->addWidget(tbForCStyle);
    tl->addStretch();
    toolsWidget->setLayout(tl);
    dockTools->setWidget(toolsWidget);
}

void MainWindow::retranslateUi()
{
    dockTools->setWindowTitle(tr("Tools"));
    tbArrow->setText(tr("Select"));
    tbProcess->setText(tr("Process / Assign"));
    tbIf->setText(tr("If...then...else"));
    tbFor->setText(tr("FOR loop"));
    tbWhilePre->setText(tr("loop with pre-condition"));
    tbWhilePost->setText(tr("loop with post-condition"));
    tbIo->setText(tr("Input"));
    tbOu->setText(tr("Output"));
    tbForCStyle->setText(tr("FOR loop (C/C++)"));
    actExit->setText(tr("E&xit"));
    actExit->setStatusTip(tr("Exit from program"));
    actOpen->setText(tr("&Open..."));
    actOpen->setStatusTip(tr("Open saved file"));
    actSave->setText(tr("&Save"));
    actSave->setStatusTip(tr("Save changes"));
    actSaveAs->setText(tr("Save &as..."));
    actSaveAs->setStatusTip(tr("Save changes in a new file"));
    actExport->setText(tr("&Export to raster..."));
    actExport->setStatusTip(tr("Save the flowchart in a raster picture format"));
    actExportSVG->setText(tr("&Export to SVG..."));
    actExportSVG->setStatusTip(tr("Save the flowchart in a vector picture format"));
    actPrint->setText(tr("&Print..."));
    actPrint->setStatusTip(tr("To print"));
    actNew->setText(tr("&New"));
    actNew->setStatusTip(tr("Create a new project"));
    actUndo->setText(tr("&Undo"));
    actUndo->setStatusTip(tr("Undo the last operation"));
    actRedo->setText(tr("&Redo"));
    actRedo->setStatusTip(tr("Restore the last undone action"));
    actCut->setText(tr("Cu&t"));
    actCut->setStatusTip(tr("Cut the current selection"));
    actCopy->setText(tr("&Copy"));
    actCopy->setStatusTip(tr("Copy the current selection"));
    actPaste->setText(tr("&Paste"));
    actPaste->setStatusTip(tr("Paste"));
    actDelete->setText(tr("&Delete"));
    actDelete->setStatusTip(tr("Delete the current selection"));
    actHelp->setText(tr("&Help"));
    actHelp->setStatusTip(tr("Open Help window"));
    actHelpv->setText(tr("&Help"));
    actHelpv->setStatusTip(tr("Close Help window"));
    actAbout->setText(tr("&About"));
    actAbout->setStatusTip(tr("Information about authors"));
    actAboutQt->setText(tr("About &Qt"));
    actAboutQt->setStatusTip(tr("Information about Qt"));
    actTools->setText(tr("&Tools"));
    actTools->setStatusTip(tr("Hide the tool panel"));
    actCode->setText(tr("&Source code"));
    actCode->setStatusTip(tr("Hide the source code panel"));
    actToolsv->setText(tr("&Tools"));
    actToolsv->setStatusTip(tr("Show the tool panel"));
    actCodev->setText(tr("&Source code"));
    actCodev->setStatusTip(tr("Show the source code panel"));
    zl->setText(tr("Zoom:"));



    actExit->setShortcut(tr("Alt+X"));
    actOpen->setShortcut(tr("Ctrl+O"));
    actSave->setShortcut(tr("Ctrl+S"));
    actNew->setShortcut(tr("Ctrl+N"));
    actUndo->setShortcut(tr("Ctrl+Z"));
    actRedo->setShortcut(tr("Ctrl+Y"));
    actCut->setShortcut(tr("Ctrl+X"));
    actCopy->setShortcut(tr("Ctrl+C"));
    actPaste->setShortcut(tr("Ctrl+V"));
    actDelete->setShortcut(tr("Del"));
    actHelp->setShortcut(tr("F1"));
    actHelpv->setShortcut(tr("F1"));
    actPrint->setShortcut(tr("Ctrl+P"));
    actToolsv->setShortcut(tr("F2"));
    actTools->setShortcut(tr("F2"));
    actCodev->setShortcut(tr("F3"));
    actCode->setShortcut(tr("F3"));

    menuFile->setTitle(tr("&File"));
    menuEdit->setTitle(tr("&Edit"));
    menuHelp->setTitle(tr("&Help"));
    menuWindow->setTitle(tr("&View"));

    toolBar->setWindowTitle(tr("Standard"));
    dockCode->setWindowTitle(tr("Source code"));
    

    int i = codeLanguage->currentIndex();
    codeLanguage->clear();
    codeLanguage->addItem(tr("Pascal"), "pas");
    codeLanguage->addItem(tr("C"), "c");
    codeLanguage->addItem(tr("C++"), "cpp");
    codeLanguage->addItem(tr("Ershov's algorithm language"), "e87");
    codeLanguage->addItem(tr("PHP"), "php");
    codeLanguage->addItem(tr("JavaScript"), "js");
    codeLanguage->addItem(tr("Python"), "pyt");
    if (i!=-1)
        codeLanguage->setCurrentIndex(i);
    else
        codeLanguage->setCurrentIndex(0);


    codeLabel->setText(tr("&Select programming language:"));

    if (!fileName.isEmpty())
        setWindowTitle(tr("%1 - Algorithm Flowchart Editor").arg(fileName));
    else
        setWindowTitle(tr("Algorithm Flowchart Editor"));
    helpWindow->setWindowTitle(tr("Help window"));

}

void MainWindow::createMenu()
{
    menuFile = menuBar()->addMenu("");
    menuFile->addAction(actNew);
    menuFile->addAction(actOpen);
    menuFile->addSeparator();
    menuFile->addAction(actSave);
    menuFile->addAction(actSaveAs);
    menuFile->addSeparator();
    menuFile->addAction(actExport);
    menuFile->addAction(actExportSVG);
    menuFile->addSeparator();
    menuFile->addAction(actPrint);
    menuFile->addSeparator();
    menuFile->addSeparator();
    menuFile->addAction(actExit);
    actAbout->isChecked();

    menuEdit = menuBar()->addMenu("");
    menuEdit->addAction(actUndo);
    menuEdit->addAction(actRedo);
    menuEdit->addSeparator();
    menuEdit->addAction(actCut);
    menuEdit->addAction(actCopy);
    menuEdit->addAction(actPaste);
    menuEdit->addSeparator();
    menuEdit->addAction(actDelete);

    menuWindow = menuBar()->addMenu("");
    menuWindow->addAction(actTools);
    menuWindow->addAction(actToolsv);
    menuWindow->addAction(actCode);
    menuWindow->addAction(actCodev);

    menuHelp = menuBar()->addMenu("");
    menuHelp->addAction(actHelp);
    menuHelp->addAction(actHelpv);
    menuHelp->addSeparator();
    menuHelp->addAction(actAbout);
    menuHelp->addAction(actAboutQt);
}

void MainWindow::createToolBar()
{
    toolBar = addToolBar("");
    toolBar->setObjectName("standard_toolbar");
    toolBar->setIconSize(QSize(32,32));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBar->addAction(actNew);
    toolBar->addAction(actOpen);
    toolBar->addAction(actSave);
    toolBar->addSeparator();
    toolBar->addAction(actUndo);
    toolBar->addAction(actRedo);
    toolBar->addSeparator();
    toolBar->addAction(actCut);
    toolBar->addAction(actCopy);
    toolBar->addAction(actPaste);
    toolBar->addSeparator();
    toolBar->addAction(actHelp);
    toolBar->addAction(actHelpv);
    toolBar->addSeparator();
    toolBar->addAction(actTools);
    toolBar->addAction(actToolsv);
    toolBar->addAction(actCode);
    toolBar->addAction(actCodev);
}

void MainWindow::helpWindowHidden()
{
    actHelp->setVisible(true);
    actHelpv->setVisible(false);
}

void MainWindow::docToolsVisibilityChanged(bool visible)
{
    if (visible) {
        actToolsv->setVisible(false);
        actTools->setVisible(true);
    } else {
        actTools->setVisible(false);
        actToolsv->setVisible(true);
    }
}

void MainWindow::docCodeVisibilityChanged(bool visible)
{
    if (visible) {
        actCodev->setVisible(false);
        actCode->setVisible(true);
    } else {
        actCode->setVisible(false);
        actCodev->setVisible(true);
    }
}

void MainWindow::createActions()
{
    actExit = new QAction(this);
    actOpen = new QAction(QIcon(":/images/open_document_32_h.png"), "", this);
    actNew = new QAction(QIcon(":/images/new_document_32_h.png"), "", this);
    actSave = new QAction(QIcon(":/images/save_32_h.png"), "", this);
    actSaveAs = new QAction(this);
    //actUndo = new QAction(QIcon(":/images/undo_32_h.png"), "", this);
    //actRedo = new QAction(QIcon(":/images/redo_32_h.png"), "", this);
    actUndo = new QAction(QIcon(":/images/restart-3.png"), "", this);
    actRedo = new QAction(QIcon(":/images/restart-4.png"), "", this);
    actCut = new QAction(QIcon(":/images/cut_clipboard_32_h.png"), "", this);
    actCopy = new QAction(QIcon(":/images/copy_clipboard_32_h.png"), "", this);
    actPaste = new QAction(QIcon(":/images/paste_clipboard_32_h.png"), "", this);
    actDelete = new QAction(QIcon(":/images/delete_x_32_h.png"), "", this);
    actExport = new QAction(this);
    actExportSVG = new QAction(this);
    actHelp = new QAction(QIcon(":/images/help_32_h.png"), "", this);
    actHelpv = new QAction(QIcon(":/images/help_32_h2.png"), "", this);
    actAbout = new QAction(this);
    actAboutQt = new QAction(this);
    actPrint = new QAction(this);
    actTools = new QAction(QIcon(":/images/tools.png"), "", this);
    actCode = new QAction(QIcon(":/images/gvim.png"), "", this);
    actToolsv = new QAction(QIcon(":/images/tools1.png"), "", this);
    actCodev = new QAction(QIcon(":/images/gvimT.png"), "", this);


    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(actNew, SIGNAL(triggered()), this, SLOT(slotFileNew()));
    connect(actOpen, SIGNAL(triggered()), this, SLOT(slotFileOpen()));
    connect(actSave, SIGNAL(triggered()), this, SLOT(slotFileSave()));
    connect(actSaveAs, SIGNAL(triggered()), this, SLOT(slotFileSaveAs()));
    connect(actExport, SIGNAL(triggered()), this, SLOT(slotFileExport()));
    connect(actExportSVG, SIGNAL(triggered()), this, SLOT(slotFileExportSVG()));
    connect(actPrint, SIGNAL(triggered()), this, SLOT(slotFilePrint()));

    connect(actCut, SIGNAL(triggered()), this, SLOT(slotEditCut()));
    connect(actCopy, SIGNAL(triggered()), this, SLOT(slotEditCopy()));
    connect(actPaste, SIGNAL(triggered()), this, SLOT(slotEditPaste()));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(slotEditDelete()));

    connect(actHelp, SIGNAL(triggered()), this, SLOT(slotHelpHelp()));
    connect(actHelpv, SIGNAL(triggered()), this, SLOT(slotHelpv()));
    connect(actAbout, SIGNAL(triggered()), this, SLOT(slotHelpAbout()));
    connect(actAboutQt, SIGNAL(triggered()), this, SLOT(slotHelpAboutQt()));
    /* ... */
    connect(actTools, SIGNAL(triggered()), this, SLOT(slotTools()));
    connect(actCode, SIGNAL(triggered()), this, SLOT(slotCode()));
    connect(actToolsv, SIGNAL(triggered()), this, SLOT(slotToolsv()));
    connect(actCodev, SIGNAL(triggered()), this, SLOT(slotCodev()));


    actCodev->setVisible(false);
    actToolsv->setVisible(false);
    actHelpv->setVisible(false);


}


MainWindow::~MainWindow()
{
}

bool MainWindow::okToContinue()
{
    int r;
    if (close()) {
        r = QMessageBox::warning(this,
                                 tr("Afce"), tr("Do you really want to close afce?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No | QMessageBox::Escape);

        if (r == QMessageBox::Yes) {
        } else if (r == QMessageBox::No) {
            return false;
        }
    }
    return true;
}

void MainWindow::slotFileOpen()
{
    QString fn = QFileDialog::getOpenFileName ( this,
                                                tr("Select a file to open"), "", tr("Algorithm flowcharts (*.afc)"));
    if(!fn.isEmpty())
    {
        emit documentUnloaded();
        fileName = fn;
        setWindowTitle(tr("%1 - Algorithm Flowchart Editor").arg(fileName));
        QFile xml(fileName);
        if (xml.exists())
        {
            xml.open(QIODevice::ReadOnly | QIODevice::Text);
            QDomDocument doc;
            if (doc.setContent(&xml, false))
            {
                document()->root()->setXmlNode(doc.firstChildElement());
                document()->setZoom(1);
            }
        }
        emit documentLoaded();
    }
}

void MainWindow::slotFilePrint()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog pd(&printer, this);
    if (pd.exec() == QDialog::Accepted)
    {
        double oldZoom = document()->zoom();
        document()->setZoom(1);
        document()->setStatus(QFlowChart::Display);
        QBlock *r = document()->root();
        r->adjustSize(1);
        r->adjustPosition(0,0);
        QRect page = printer.pageRect();
        double z =  page.width()  / (double) r->width;
        if (r->height * z > page.height())
        {
            z = page.height() / (double) r->height;
        }

        document()->setZoom(z);
        r->adjustSize(z);
        r->adjustPosition(0, 0);
        QPainter canvas;
        canvas.begin(&printer);
        document()->paintTo(&canvas);
        canvas.end();
        document()->setZoom(oldZoom);
        document()->setStatus(QFlowChart::Selectable);
    }
}

void MainWindow::slotFileNew()
{
    QProcess::startDetached(QApplication::applicationFilePath());
}

void MainWindow::slotFileSave()
{
    if (fileName.isEmpty())
    {
        slotFileSaveAs();
    }
    else
    {
        QDomDocument doc = document()->document();
        QString xmlString = doc.toString(2);
        QFile xml(fileName);
        xml.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
        QTextStream stream(&xml);
        stream.setCodec(QTextCodec::codecForName("utf-8"));
        stream << xmlString;
        xml.close();
        emit documentSaved();
    }
}

void MainWindow::slotFileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Select a file to save"), "", tr("Algorithm flowcharts (*.afc)"));
    if (!fn.isEmpty())
    {
        fileName = fn;
        setWindowTitle(tr("%1 - Algorithm Flowchart Editor").arg(fileName));
        slotFileSave();
    }
}

void MainWindow::slotFileExport()
{
    QString filter = getWriteFormatFilter();
    QString fn = QFileDialog::getSaveFileName(this, tr("Select a file to export"), "", filter);
    if(!fn.isEmpty())
    {
        double oldZoom = document()->zoom();
        document()->setZoom(1);
        document()->setStatus(QFlowChart::Display);
        QBlock *r = document()->root();
        r->adjustSize(1);
        r->adjustPosition(0,0);
        QImage img(r->width, r->height, QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter canvas(&img);
        canvas.setRenderHint(QPainter::Antialiasing);
        document()->paintTo(&canvas);
        img.save(fn);
        document()->setZoom(oldZoom);
        document()->setStatus(QFlowChart::Selectable);
    }
}

void MainWindow::slotFileExportSVG()
{
    QString filter = getWriteFormatFilter();
    QString fn = QFileDialog::getSaveFileName(this, tr("Select a file to export"), "", getFilterFor("svg"));
    if(!fn.isEmpty())
    {
        double oldZoom = document()->zoom();
        document()->setZoom(1);
        document()->setStatus(QFlowChart::Display);
        QBlock *r = document()->root();
        r->adjustSize(1);
        r->adjustPosition(0,0);
        QSvgGenerator svg;
        svg.setSize(QSize(r->width, r->height));
        svg.setResolution(90);
        svg.setFileName(fn);
        QPainter canvas(&svg);
        canvas.setRenderHint(QPainter::Antialiasing);
        r->paint(&canvas, true);

        document()->setZoom(oldZoom);
        document()->setStatus(QFlowChart::Selectable);
    }
}


void MainWindow::slotEditCut()
{
    slotEditCopy();
    slotEditDelete();
}

void MainWindow::slotEditCopy()
{
    if(document())
    {
        if(document()->activeBlock())
        {
            QDomDocument doc("AFC"); // do not localize!
            QDomElement block = document()->activeBlock()->xmlNode(doc);
            if (document()->activeBlock()->isBranch)
            {
                QDomElement alg = doc.createElement("algorithm");
                alg.appendChild(block);
                doc.appendChild(alg);
            }
            else
            {
                if(block.nodeName() != "algorithm")
                {
                    QDomElement alg = doc.createElement("algorithm");
                    QDomElement branch = doc.createElement("branch");
                    alg.appendChild(branch);
                    branch.appendChild(block);
                    doc.appendChild(alg);
                }
                else
                    doc.appendChild(block);
            }
            QClipboard *clipbrd = QApplication::clipboard();
            clipbrd->setText(doc.toString(2));
            updateActions();
        }
    }
}

void MainWindow::slotEditPaste()
{
    if(document())
    {
        QClipboard *clipbrd = QApplication::clipboard();
        document()->setBuffer(clipbrd->text());
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}
void MainWindow::slotEditDelete()
{
    if(document())
    {
        if(document()->status() == QFlowChart::Selectable)
        {
            document()->deleteActiveBlock();
        }
    }
}

void MainWindow::slotHelpHelp()
{
    helpWindow->show();
    actHelp->setVisible(false);
    actHelpv->setVisible(true);
}
void MainWindow::slotHelpv()
{
    helpWindow->hide();
    actHelp->setVisible(true);
    actHelpv->setVisible(false);
}
void MainWindow::slotTools()                          /* *Icon tools, code* */
{
    bool sigstate = dockTools->blockSignals(true);

    dockTools->hide();
    dockTools->blockSignals(sigstate);
    actTools->setVisible(false);
    actToolsv->setVisible(true);
}
void MainWindow::slotCode()
{
    bool sigstate = dockCode->blockSignals(true);
    dockCode->hide();
    dockCode->blockSignals(sigstate);
    actCode->setVisible(false);
    actCodev->setVisible(true);


}
void MainWindow::slotToolsv()
{
    bool sigstate = dockTools->blockSignals(true);
    dockTools->show();
    dockTools->blockSignals(sigstate);
    actToolsv->setVisible(false);
    actTools->setVisible(true);
}
void MainWindow::slotCodev()
{
    bool sigstate = dockCode->blockSignals(true);
    dockCode->show();
    dockCode->blockSignals(sigstate);
    actCode->setVisible(true);
    actCodev->setVisible(false);

}

void MainWindow::slotHelpAbout()
{
    QDialog dlg;
    QPushButton *ok = new QPushButton(tr("&OK"));
    QLabel *text = new QLabel(tr("<html><h1>AFCE</h1><p>Algorithm Flowchart Editor</p><p>Copyright 2008-2014 Viktor Zinkevich. All rights reserved.</p> \
<p>Contributors:  Sergey Ryabenko, Alexey Loginov</p> \
<p>The program is provided AS IS with NO WARRANTY OF ANY KIND,<br> INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND<br> \
FITNESS FOR A PARTICULAR PURPOSE.</p></html>"));
    QLabel *ico = new QLabel();
    ico->setPixmap(QPixmap(":/images/icon.png"));
    QGridLayout *layout = new QGridLayout;
    QHBoxLayout *bl = new QHBoxLayout;
    bl->addStretch();
    bl->addWidget(ok);
    layout->addWidget(ico, 0, 0, 2, 1, Qt::AlignTop);
    layout->addWidget(text, 0, 1);
    layout->addLayout(bl, 1, 1);
    dlg.setLayout(layout);
    connect(ok, SIGNAL(clicked()), &dlg, SLOT(accept()));
    dlg.exec();
}

void MainWindow::slotHelpAboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::slotStatusChanged()
{
    if(document())
    {
        if(document()->status() == QFlowChart::Selectable)
        {
            tbArrow->setChecked(true);
        }
    }
}

void MainWindow::updateActions()
{
    if(document())
    {
        if (actTools->isVisible())
        {
            actTools->setEnabled(false);
        }



        actCode->setEnabled(document()->status() != QFlowChart::Insertion);
        actTools->setEnabled(document()->status() != QFlowChart::Insertion);
        actHelp->setEnabled(document()->status() != QFlowChart::Insertion);
        actCodev->setEnabled(document()->status() != QFlowChart::Insertion);
        actToolsv->setEnabled(document()->status() != QFlowChart::Insertion);
        actHelpv->setEnabled(document()->status() != QFlowChart::Insertion);
        actUndo->setEnabled(document()->canUndo() && document()->status() != QFlowChart::Insertion);
        actRedo->setEnabled(document()->canRedo() && document()->status() != QFlowChart::Insertion);
        actOpen->setEnabled(document()->status() != QFlowChart::Insertion);
        actSave->setEnabled(document()->status() != QFlowChart::Insertion);
        actSaveAs->setEnabled(document()->status() != QFlowChart::Insertion);
        actPrint->setEnabled(document()->status() != QFlowChart::Insertion);
        actExport->setEnabled(document()->status() != QFlowChart::Insertion);
        actExportSVG->setEnabled(document()->status() != QFlowChart::Insertion);
        actCopy->setEnabled(document()->status() == QFlowChart::Selectable && document()->activeBlock());
        actCut->setEnabled(document()->status() == QFlowChart::Selectable && document()->activeBlock());
        actPaste->setEnabled(document()->status() == QFlowChart::Selectable && document()->canPaste());
        actDelete->setEnabled(document()->status() == QFlowChart::Selectable && document()->activeBlock());
    }
    else
    {
        actUndo->setEnabled(false);
        actRedo->setEnabled(false);
        actOpen->setEnabled(false);
        actSave->setEnabled(false);
        actSaveAs->setEnabled(false);
        actPrint->setEnabled(false);
        actExport->setEnabled(false);
        actExportSVG->setEnabled(false);
        actCopy->setEnabled(false);
        actCut->setEnabled(false);
        actPaste->setEnabled(false);
        actDelete->setEnabled(false);
    }
}

void MainWindow::codeLangChanged(int )
{
    generateCode();
}

void MainWindow::generateCode()
{
    if (document())
    {
        switch (codeLanguage->currentIndex())
        {
        case 0:
        {
            codeText->setText(xmlToPascal(document()->document()));
            break;
        }
        case 1:
        {
            codeText->setText(xmlToCdef(document()->document()));
            break;
        }
        case 2:
        {
            codeText->setText(xmlToC(document()->document()));
            break;
        }
        case 3:
        {
            codeText->setText(xmlToE87(document()->document()));
            break;
        }
        case 4:
        {
            codeText->setText(xmlToPHP(document()->document()));
            break;
        }
        case 5:
        {
            codeText->setText(xmlToJavaScript(document()->document()));
            break;
        }
        case 6:
        {
            codeText->setText(xmlToPython(document()->document()));
            break;
        }

        }
    }
}

void MainWindow::slotEditBlock(QBlock *aBlock)
{
    if(aBlock)
    {
        QDialog dlg;
        QVBoxLayout *mainLayout = new QVBoxLayout();
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *btnOk = new QPushButton(tr("&OK"));
        QPushButton *btnCancel = new QPushButton(tr("&Cancel"));
        connect(btnOk, SIGNAL(clicked()), &dlg, SLOT(accept()));
        connect(btnCancel, SIGNAL(clicked()), &dlg, SLOT(reject()));
        buttonLayout->addStretch();
        buttonLayout->addWidget(btnOk);
        buttonLayout->addWidget(btnCancel);
        dlg.setLayout(mainLayout);
        if(aBlock->type() == "process" || aBlock->type() == "if" || aBlock->type() == "pre" || aBlock->type() == "post" )
        {

            QLineEdit *text = new QLineEdit();
            QLabel *lab = new QLabel(tr("&Content:"));
            QString attr = "text";
            if (aBlock->type() == "process")
            {
                dlg.setWindowTitle(tr("Process"));
            }
            else if (aBlock->type() == "if")
            {
                dlg.setWindowTitle(tr("Branching"));
                lab->setText(tr("&Condition:"));
                attr = "cond";
            }
            else if (aBlock->type() == "pre")
            {
                dlg.setWindowTitle(tr("WHILE loop"));
                lab->setText(tr("&Condition:"));
                attr = "cond";
            }
            else if (aBlock->type() == "post")
            {
                dlg.setWindowTitle(tr("Post-condition loop"));
                lab->setText(tr("&Condition:"));
                attr = "cond";
            }
            text->setText(aBlock->attributes.value(attr, ""));
            lab->setBuddy(text);
            QHBoxLayout *box = new QHBoxLayout;
            box->addWidget(lab);
            box->addWidget(text);
            mainLayout->addLayout(box);
            mainLayout->addLayout(buttonLayout);

            if (dlg.exec() == QDialog::Accepted)
            {
                aBlock->attributes[attr] = text->text();
                if(aBlock->flowChart())
                {
                    aBlock->flowChart()->update();
                    aBlock->flowChart()->makeChanged();
                }
            }
        }
        else if(aBlock->type() == "for")
        {
            dlg.setWindowTitle(tr("FOR loop"));
            QGridLayout *gl = new QGridLayout();
            QLineEdit *teVar = new QLineEdit();
            QLineEdit *teFrom = new QLineEdit();
            QLineEdit *teTo = new QLineEdit();
            QLabel *labVar = new QLabel(tr("&Variable:"));
            QLabel *labFrom = new QLabel(tr("&Start value:"));
            QLabel *labTo = new QLabel(tr("&End value:"));
            gl->addWidget(labVar, 0,0);
            gl->addWidget(teVar, 0,1);
            gl->addWidget(labFrom, 1,0);
            gl->addWidget(teFrom, 1,1);
            gl->addWidget(labTo, 2,0);
            gl->addWidget(teTo, 2,1);
            labFrom->setBuddy(teFrom);
            labTo->setBuddy(teTo);
            labVar->setBuddy(teVar);

            mainLayout->addLayout(gl);
            mainLayout->addLayout(buttonLayout);
            teVar->setText(aBlock->attributes.value("var", ""));
            teFrom->setText(aBlock->attributes.value("from", ""));
            teTo->setText(aBlock->attributes.value("to", ""));
            if (dlg.exec() == QDialog::Accepted)
            {
                aBlock->attributes["var"] = teVar->text();
                aBlock->attributes["from"] = teFrom->text();
                aBlock->attributes["to"] = teTo->text();
                if(aBlock->flowChart())
                {
                    aBlock->flowChart()->update();
                    aBlock->flowChart()->makeChanged();
                }
            }
        }
        else if(aBlock->type() == "io" || aBlock->type() == "ou")
        {
            if (aBlock->type() == "io")
            {
                    dlg.setWindowTitle(tr("Input"));
                }
            else if (aBlock->type() == "ou")
            {
                    dlg.setWindowTitle(tr("Output"));
                }
            QGridLayout *gl = new QGridLayout();
            QLineEdit *t1 = new QLineEdit();
            QLineEdit *t2 = new QLineEdit();
            QLineEdit *t3 = new QLineEdit();
            QLineEdit *t4 = new QLineEdit();
            QLineEdit *t5 = new QLineEdit();
            QLineEdit *t6 = new QLineEdit();
            QLineEdit *t7 = new QLineEdit();
            QLineEdit *t8 = new QLineEdit();
            QLabel *l1;
            QLabel *l2;
            QLabel *l3;
            QLabel *l4;
            QLabel *l5;
            QLabel *l6;
            QLabel *l7;
            QLabel *l8;
            QLabel *numer;
            QLabel *cont;
            numer = new QLabel(tr("#"));
            l1 = new QLabel(tr("1:"));
            l2 = new QLabel(tr("2:"));
            l3 = new QLabel(tr("3:"));
            l4 = new QLabel(tr("4:"));
            l5 = new QLabel(tr("5:"));
            l6 = new QLabel(tr("6:"));
            l7 = new QLabel(tr("7:"));
            l8 = new QLabel(tr("8:"));

            cont = new QLabel(tr("Content:"));

            gl->addWidget(numer, 0,0);
            gl->addWidget(cont, 0,1);
            gl->addWidget(l1, 1,0);
            gl->addWidget(t1, 1,1);
            gl->addWidget(l2, 2,0);
            gl->addWidget(t2, 2,1);
            gl->addWidget(l3, 3,0);
            gl->addWidget(t3, 3,1);
            gl->addWidget(l4, 4,0);
            gl->addWidget(t4, 4,1);
            gl->addWidget(l5, 5,0);
            gl->addWidget(t5, 5,1);
            gl->addWidget(l6, 6,0);
            gl->addWidget(t6, 6,1);
            gl->addWidget(l7, 7,0);
            gl->addWidget(t7, 7,1);
            gl->addWidget(l8, 8,0);
            gl->addWidget(t8, 8,1);
            l1->setBuddy(t1);
            l2->setBuddy(t2);
            l3->setBuddy(t3);
            l4->setBuddy(t4);
            l5->setBuddy(t5);
            l6->setBuddy(t6);
            l7->setBuddy(t7);
            l8->setBuddy(t8);

            mainLayout->addLayout(gl);
            mainLayout->addLayout(buttonLayout);
            t1->setText(aBlock->attributes.value("t1", ""));
            t2->setText(aBlock->attributes.value("t2", ""));
            t3->setText(aBlock->attributes.value("t3", ""));
            t4->setText(aBlock->attributes.value("t4", ""));
            t5->setText(aBlock->attributes.value("t5", ""));
            t6->setText(aBlock->attributes.value("t6", ""));
            t7->setText(aBlock->attributes.value("t7", ""));
            t8->setText(aBlock->attributes.value("t8", ""));


            if (dlg.exec() == QDialog::Accepted)
            {
                aBlock->attributes["t1"] = t1->text();
                aBlock->attributes["t2"] = t2->text();
                aBlock->attributes["t3"] = t3->text();
                aBlock->attributes["t4"] = t4->text();
                aBlock->attributes["t5"] = t5->text();
                aBlock->attributes["t6"] = t6->text();
                aBlock->attributes["t7"] = t7->text();
                aBlock->attributes["t8"] = t8->text();


                if(aBlock->flowChart())
                {
                    aBlock->flowChart()->update();
                    aBlock->flowChart()->makeChanged();
                }
            }
        }

        else if(aBlock->type() == "assign")
        {
            dlg.setWindowTitle(tr("Assign"));
            QGridLayout *gl = new QGridLayout();
            QLineEdit *leSrc = new QLineEdit();
            QLineEdit *leDest = new QLineEdit();
            QLabel *labSrc = new QLabel(tr("&Source:"));
            QLabel *labDest = new QLabel(tr("&Destination:"));
            gl->addWidget(labDest, 0,0);
            gl->addWidget(leDest, 0,1);
            gl->addWidget(labSrc, 1,0);
            gl->addWidget(leSrc, 1,1);
            labDest->setBuddy(leDest);
            labSrc->setBuddy(leSrc);

            mainLayout->addLayout(gl);
            mainLayout->addLayout(buttonLayout);
            leSrc->setText(aBlock->attributes.value("src", ""));
            leDest->setText(aBlock->attributes.value("dest", ""));

            if (dlg.exec() == QDialog::Accepted)
            {
                aBlock->attributes["dest"] = leDest->text();
                aBlock->attributes["src"] = leSrc->text();

                if(aBlock->flowChart())
                {
                    aBlock->flowChart()->update();
                    aBlock->flowChart()->makeChanged();
                }
            }
        }

    }
}

void MainWindow::slotToolAssing()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><assign dest=\"x\" src=\"0\"/></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}

void MainWindow::slotToolArrow()
{
    if(document())
    {
        document()->setStatus(QFlowChart::Selectable);
        document()->update();
    }
}
void MainWindow::slotToolProcess()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><process text=\"z = x + y\"/></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}

void MainWindow::slotToolIf()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><if cond=\"x &gt; 0\"><branch /><branch /></if></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}

void MainWindow::slotToolFor()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><for var=\"i\" from=\"0\" to=\"n - 1\"><branch /></for></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}

void MainWindow::slotToolWhilePre()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><pre cond=\"x &lt; n\"><branch /></pre></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }
}

void MainWindow::slotToolWhilePost()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><post cond=\"x &lt; n\"><branch /></post></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }

}

void MainWindow::slotToolIo()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><io t1=\"x\" t2=\"\" t3=\"\" t4=\"\" t5=\"\" t6=\"\" t7=\"\" t8=\"\"/></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }

}




void MainWindow::slotToolOu()
{
    if(document())
    {
        document()->setBuffer("<algorithm><branch><ou t1=\"z\" t2=\"\" t3=\"\" t4=\"\" t5=\"\" t6=\"\" t7=\"\" t8=\"\"/></branch></algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }

}

void MainWindow::slotToolCase()
{
    //  if(document())
    //  {
    //    document()->setBuffer("<algorithm><branch><case><branch /><branch /><branch /></case></branch></algorithm>");
    //    if(!document()->buffer().isEmpty())
    //    {
    //      document()->setStatus(QFlowChart::Insertion);
    //      document()->setMultiInsert(false);
    //    }
    //  }

}

void MainWindow::slotToolForCStyle()
{
    if(document())
    {
        document()->setBuffer("<algorithm> <branch> <assign dest=\"i\" src=\"0\"  /> <pre cond=\"i &lt; n\"> <branch> <assign dest=\"i\" src=\"i + 1\" /> </branch> </pre> </branch> </algorithm>");
        if(!document()->buffer().isEmpty())
        {
            document()->setStatus(QFlowChart::Insertion);
            document()->setMultiInsert(false);
        }
    }

}

QString MainWindow::getFilterFor(const QString & fileExt)
{
    return tr("%1 image (*.%2)").arg(fileExt.toUpper(), fileExt);
}

QString MainWindow::getWriteFormatFilter()
{
    QString result;
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    for(int i = 0; i < formats.size(); ++i)
    {
        if(!result.isEmpty()) result.append(";;");
        result.append(getFilterFor(formats.at(i)));
    }
    return result;
}

void MainWindow::setDocument(QFlowChart * aDocument)
{
    fDocument = aDocument;
    saScheme->setWidget(fDocument);
    saScheme->setAutoFillBackground(true);
    fDocument->show();
    fDocument->move(0,0);
}
void MainWindow::setZoom(int Percents)
{
    zoomLabel->setText(tr("Zoom: %1 %").arg(Percents));
    if (document())
    {
        document()->setZoom(Percents / 100.0);
    }
}
