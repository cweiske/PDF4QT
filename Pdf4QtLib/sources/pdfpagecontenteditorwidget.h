//    Copyright (C) 2022 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFPAGECONTENTEDITORWIDGET_H
#define PDFPAGECONTENTEDITORWIDGET_H

#include "pdfglobal.h"

#include <QDockWidget>
#include <QSignalMapper>

class QToolButton;

namespace Ui
{
class PDFPageContentEditorWidget;
}

namespace pdf
{

class PDF4QTLIBSHARED_EXPORT PDFPageContentEditorWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit PDFPageContentEditorWidget(QWidget* parent);
    virtual ~PDFPageContentEditorWidget() override;

    /// Adds external action to the tool box
    void addAction(QAction* action);

    QToolButton* getToolButtonForOperation(int operation) const;

signals:
    void operationTriggered(int operation);

private:
    void onActionTriggerRequest(QObject* actionObject);
    void onActionChanged();

    Ui::PDFPageContentEditorWidget* ui;
    QSignalMapper m_actionMapper;
    QSignalMapper m_operationMapper;
    int m_toolBoxColumnCount;
    QSize m_toolButtonIconSize;
};

}   // namespace pdf

#endif // PDFPAGECONTENTEDITORWIDGET_H
