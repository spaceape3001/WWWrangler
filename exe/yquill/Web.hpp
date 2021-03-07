////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <dbgui/web/WebBase.hpp>

class Web : public WebBase {
    Q_OBJECT
public:
    Web(QWidget*parent=nullptr);
    ~Web();
private slots:
    void    update_title();
};
