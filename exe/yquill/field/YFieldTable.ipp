////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

YFieldTable::YFieldTable(FieldProvider stp, QWidget*parent) : SubWin(parent), m_model{}, m_view{}, m_addKey{}, m_addBtn{}, m_addEnable{}
{
    if(!stp)
        stp     = provider::all_fields();
        
    m_model     = new FieldTableModel(stp, this);
    
    m_model -> setReadOnly(false);
    m_view      = new FieldTableView(m_model);
    
    QToolBar*   tb  = new QToolBar;
    m_addKey        = new QLineEdit;
    m_addKey -> setValidator(new KeyValidator(m_addKey));
    m_addBtn        = new QPushButton("+");
    setAddEnable(false);
    
    connect(m_addKey, &QLineEdit::returnPressed, this, &YFieldTable::cmdAdd);
    connect(m_addBtn, &QPushButton::clicked, this, &YFieldTable::cmdAdd);
    
    connect(m_view, &QTableView::doubleClicked, this, &YFieldTable::doubleClicked);
    
    tb->addWidget(m_addKey);
    tb->addWidget(m_addBtn);
    
    QVBoxLayout*    lay = new QVBoxLayout(this);
    lay -> addWidget(m_view, 1);
    lay -> addWidget(tb, 0);
    
    lay -> setContentsMargins(0,0,0,0);
    
    setWindowTitle("Fields");
}

YFieldTable::~YFieldTable()
{
}

void    YFieldTable::check() 
{
    m_model -> check();
}

void      YFieldTable::cmdAdd()
{
    if(m_addEnable){
        //QString txt = m_addKey -> text().simplified();
        //if(txt.isEmpty())
            //return ;
        //Field t   = cdb::make_tag(txt);
        //if(t)
            //m_model -> append(t);
    }
    
    m_addKey -> clear();
}

void      YFieldTable::doubleClicked(const QModelIndex&idx)
{
    if(is_in({"Key", "ID"}, m_model->columnKey(idx))){
        Field t   = m_model -> get(idx.row());
        if(t)
            emit editReq(t.id);
    }
}

void            YFieldTable::refresh()
{
    m_model -> refresh();
}

void            YFieldTable::setAddEnable(bool f)
{
    m_addEnable = f;
    m_addKey -> setEnabled(f);
    m_addBtn -> setEnabled(f);
}


void   YFieldTable::setReadOnly(bool f)
{
    m_model -> setReadOnly(f);
}



