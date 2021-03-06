////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dbgui/folder/FolderExplorerBase.hpp"

class YExplorer : public FolderExplorerBase {
    Q_OBJECT
public:
    YExplorer(QWidget* parent=nullptr);
    ~YExplorer();
    
private slots:
    void        updateTitle();
};
