////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#include "FolderExplorerBase.hpp"
#include "FolderExplorerImpl.hpp"

#include <db/Workspace.hpp>
#include <gui/StdIcons.hpp>
#include <util/DiffEngine.hpp>
#include <util/Reverse.hpp>
#include <util/Vector.hpp>

#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QWheelEvent>

#include <cmath>

static_assert(sizeof(void*) >= sizeof(uint64_t), "MUST BE at laest 64-bit code!");

template class StdListModel<FolderExplorerBase::Entry>;
template class StdListView<FolderExplorerBase::Entry>;
template class StdTableModel<FolderExplorerBase::Entry>;
template class StdTableView<FolderExplorerBase::Entry>;

namespace {
    QIcon   icon_for(Document d)
    {
        QIcon ret    = cdb::qIcon(cdb::icon(d));
        if(ret.isNull())
            ret    = si_view_refresh();    // placeholder
        return ret;
    }

    QIcon   icon_for(Folder f)
    {
        return si_folder();
    }
    
}


FolderExplorerBase::FolderExplorerBase(QWidget*parent) : SubWin(parent)
{
    m_url       = new QComboBox;
    m_url -> setEditable(true);
    m_url -> setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    
    m_listModel     = new ListModel(this);
    m_listView      = new ListView(m_listModel);
    m_tableModel    = new TableModel(this);
    m_tableView     = new TableView(m_tableModel);
    m_treeModel     = new TreeModel(this);
    m_treeView      = new TreeView(m_treeModel);
    
    m_stacked       = new QStackedWidget;
    m_listPos       = m_stacked -> addWidget(m_listView);
    m_tablePos      = m_stacked -> addWidget(m_tableView);
    
    m_stacked -> setCurrentWidget(m_listView);

    QToolBar*       bar = new QToolBar;
    m_backAct       = bar -> addAction(si_go_back(), tr("Back"), this, &FolderExplorerBase::cmdGoBack);
    m_forwardAct    = bar -> addAction(si_go_forward(), tr("Fwd"), this, &FolderExplorerBase::cmdGoForward);
    m_upAct         = bar -> addAction(si_go_up(), tr("Up"), this, &FolderExplorerBase::cmdGoUp);
    
    bar -> addAction(si_view_refresh(), tr("Reload"), this, &FolderExplorerBase::cmdReload);
    //bar -> addAction(si_go_home(), tr("Home"), this, &WebBase::goHome);
    bar -> addWidget(m_url);
    
    connect(m_listView, &FolderExplorerBase::ListView::dblClickFolder, this, &FolderExplorerBase::cmdNavigateTo);
    connect(m_tableView, &FolderExplorerBase::TableView::dblClickFolder, this, &FolderExplorerBase::cmdNavigateTo);
    connect(m_treeView, &FolderExplorerBase::TreeView::activatedFolder, this, &FolderExplorerBase::cmdNavigateTo);
    connect(m_listView, &FolderExplorerBase::ListView::dblClickDoc, this, &FolderExplorerBase::openRequest);
    connect(m_tableView, &FolderExplorerBase::TableView::dblClickDoc, this, &FolderExplorerBase::openRequest);
    
    QSplitter   *split  = new QSplitter(Qt::Horizontal);
    split -> addWidget(m_treeView);
    split -> addWidget(m_stacked);
    split -> setStretchFactor(0, 1);
    split -> setStretchFactor(1, 4);
    
    QVBoxLayout*    vlay    = new QVBoxLayout(this);
    vlay -> addWidget(bar, 0);
    vlay -> addWidget(split, 1);
    vlay -> setContentsMargins(0,0,0,0);
    
}

FolderExplorerBase::~FolderExplorerBase()
{
}

void    FolderExplorerBase::changeTo(Folder f)
{
    if(f == m_folder)
        return ;
    
    m_folder    = f;
    m_treeView -> setCurrentFolder(f);
    m_listModel -> refresh();
    m_tableModel -> refresh();
    m_upAct -> setEnabled( f != cdb::top_folder() );
    m_backAct -> setEnabled( !m_history.empty() && m_histPos );
    m_forwardAct -> setEnabled( (m_histPos >= 0) && (m_histPos < (int) m_history.size()));
}

void    FolderExplorerBase::cmdGoBack()
{
    if(m_history.empty())
        return ;
    m_history << m_folder;
    if(m_histPos < 0){
        m_histPos   = (int) m_history.size() - 2;
    } else if(m_histPos > 0){
        --m_histPos;
    } else
        return ;
        
    changeTo(m_history[m_histPos]);
}

void    FolderExplorerBase::cmdGoForward()
{
    if(m_history.empty())
        return ;
    if(m_histPos < 0)
        return;
    if(m_histPos >= (int) m_history.size() - 1)
        return;
    ++m_histPos;
    changeTo(m_history[m_histPos]);
}

void    FolderExplorerBase::cmdGoUp()
{
    cmdNavigateTo(cdb::parent(m_folder));
}

void    FolderExplorerBase::cmdNavigateTo(Folder f)
{
    if(!f || (f == m_folder))
        return ;

    if(m_histPos >= 0){
        m_history.pop_to(m_histPos);
        m_histPos   = -1;
    }
    if(m_folder)
        m_history << m_folder;
        
    changeTo(f);
}

void    FolderExplorerBase::cmdReload()
{
}

void    FolderExplorerBase::refresh()
{
    m_treeModel -> refresh();
    m_tableModel -> refresh();
    m_listModel -> refresh();
}

unsigned FolderExplorerBase::searchOpts() const
{
    return (m_showHidden ? cdb::Hidden : 0) | cdb::BestSort;
}

void    FolderExplorerBase::setShowHidden(bool f)
{
    if(f != m_showHidden){
        m_showHidden        = f;
        refresh();
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<FolderExplorerBase::Entry>    FolderExplorerBase::makeEntries(const Vector<Document>& items)
{
    return Vector<Entry>(items.begin(), items.end());
}

Vector<FolderExplorerBase::Entry>    FolderExplorerBase::makeEntries(const Vector<Folder>& items)
{
    return Vector<Entry>(items.begin(), items.end());
}

Vector<FolderExplorerBase::Entry>    FolderExplorerBase::makeEntries(const Vector<cdb::DocOrFold>& items)
{
    return Vector<Entry>(items.begin(), items.end());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FolderExplorerBase::Entry::Entry(Folder f) 
{
    set(f);
}

FolderExplorerBase::Entry::Entry(Document d)
{
    set(d);
}

FolderExplorerBase::Entry::Entry(cdb::DocOrFold df)
{
    Document*   d = std::get_if<Document>(&df);
    if(d)
        set(*d);
    Folder*     f = std::get_if<Folder>(&df);
    if(f)
        set(*f);
}

void  FolderExplorerBase::Entry::set(Document d)
{
    type    = Type::Doc;
    id      = d.id;
    icon    = icon_for(d);
    label   = cdb::skey(d);
}

void  FolderExplorerBase::Entry::set(Folder f)
{
    type    = Type::Fold;
    id      = f.id;
    icon    = icon_for(f);
    label   = cdb::skey(f);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct FolderExplorerBase::NameCol : public StdColumn<Entry> {
    
    QVariant display(const Entry& e) const override
    {
        return e.label;
    }
    
    QVariant decoration(const Entry& e) const override
    {
        return e.icon;
    } 
    
    NameCol() : StdColumn<Entry>("Name") {}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FolderExplorerBase::ListModel::ListModel(FolderExplorerBase* parent) : StdListModel<Entry>(parent), m_fx(parent)
{
    col(new NameCol);
}

FolderExplorerBase::ListModel::~ListModel()
{
}

void  FolderExplorerBase::ListModel::refresh()
{
    unsigned int        opts    = m_fx -> searchOpts();
    setAllData(makeEntries(cdb::folders(m_fx->m_folder, opts)) + makeEntries(cdb::documents(m_fx->m_folder, opts)));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const double kZoomDelta  = pow(2.0, 1.0/8.0);
static const double kZoomMax    = 4.0;
static const double kZoomMin    = pow(0.5, 2.);


FolderExplorerBase::ListView::ListView(ListModel* lm, QWidget* parent) : StdListView<Entry>(lm, parent), m_model(lm)
{
    setViewMode(IconMode);
    connect(this, &QListView::doubleClicked, this, &FolderExplorerBase::ListView::cmdDoubleClicked);
}

FolderExplorerBase::ListView::~ListView()
{
}

void    FolderExplorerBase::ListView::applyLayout()
{
    int     iz  = (int)(0.5 + std::max(16., m_zoom*48.0));
    setIconSize(QSize(iz, iz));
    
    //  TODO: Make this a uniform grid based on contents
}

void    FolderExplorerBase::ListView::cmdDoubleClicked(const QModelIndex& idx)
{
    const Entry*    e   = m_model -> row(idx);
    if(e){
        switch(e->type){
        case Type::Doc:
            emit dblClickDoc(Document{e->id});
            break;
        case Type::Fold:
            emit dblClickFolder(Folder{e->id});
            break;
        default:
            break;
        }
    }
}


void    FolderExplorerBase::ListView::resizeEvent(QResizeEvent*evt)
{
    QListView::resizeEvent(evt);
    applyLayout();
}

void    FolderExplorerBase::ListView::showEvent(QShowEvent*evt)
{
    QListView::showEvent(evt);
    if(m_firstShow){
        m_firstShow = false;
        applyLayout();
    }
}


void    FolderExplorerBase::ListView::wheelEvent(QWheelEvent*evt)
{
    if(evt->modifiers() & Qt::ControlModifier){
        zoomBy( pow(kZoomDelta, evt->angleDelta().y() / 120.));
        evt -> accept();
        return;
    } 
    return QListView::wheelEvent(evt);
}

void    FolderExplorerBase::ListView::zoomBy(double amt)
{
    
    m_zoom  = std::min(kZoomMax, std::max(kZoomMin, m_zoom * amt));
    applyLayout();
}


void    FolderExplorerBase::ListView::zoomIn()
{
    zoomBy(kZoomDelta);
}

void    FolderExplorerBase::ListView::zoomOut()
{
    zoomBy(1./kZoomDelta);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FolderExplorerBase::TableModel::TableModel(FolderExplorerBase* parent) : StdTableModel<Entry>(parent), m_fx(parent)
{
    col(new NameCol);
}

FolderExplorerBase::TableModel::~TableModel()
{
}

void  FolderExplorerBase::TableModel::refresh()
{
    unsigned int        opts    = m_fx -> searchOpts();
    setAllData(makeEntries(cdb::folders(m_fx -> m_folder, opts)) + makeEntries(cdb::documents(m_fx->m_folder, opts)));
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FolderExplorerBase::TableView::TableView(TableModel*tm, QWidget*parent) : StdTableView<Entry>(tm, parent), m_model(tm)
{
    connect(this, &QTableView::doubleClicked, this, &FolderExplorerBase::TableView::cmdDoubleClicked);
}

FolderExplorerBase::TableView::~TableView()
{
}

void        FolderExplorerBase::TableView::cmdDoubleClicked(const QModelIndex&idx)
{
    const Entry*    e   = m_model -> row(idx);
    if(e){
        switch(e->type){
        case Type::Doc:
            emit dblClickDoc(Document{e->id});
            break;
        case Type::Fold:
            emit dblClickFolder(Folder{e->id});
            break;
        default:
            break;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct FolderExplorerBase::TreeModel::Node {
    TreeModel*              const  ftm;
    const Folder            folder;
    const Folder            parent;
    const QString           label;
    Image                   icoImg;
    QIcon                   icon;
    Vector<Folder>          children;
    
    enum {
        CHG_DATA            = 1,
        CHG_CHILDREN        = 2
    };
    
    
    Node(TreeModel* m, Folder f) : ftm(m), folder(f), parent(cdb::parent(f)), 
        label((f == cdb::top_folder()) ? wksp::name() : cdb::skey(f)), icon(si_folder())
    {
    }
    
    void    refresh(unsigned chgSignal = 3)
    {
        Vector<Folder>  nc  = cdb::folders(folder, ftm->m_fx->searchOpts());
        if(chgSignal&CHG_CHILDREN){
            auto chg    = diff_it(children, nc);
            if(any_changes(chg)){
                QModelIndex par = ftm->index(parent);
            
                for(auto& de : reverse(chg)){
                    if(de.mode == DEResult::Remove){
                        ftm->beginRemoveRows(par, de.left.low, de.left.high-1);
                        children.erase_range(de.left.low, de.left.high);
                        ftm->endRemoveRows();
                    }
                }
                
                for(auto& de : chg){
                    if(de.mode == DEResult::Add){
                        ftm->beginInsertRows(par, de.right.low, de.right.high-1);
                        children.insert(children.begin() + de.right.low, nc.begin()+de.right.low, nc.begin()+de.right.high);
                        ftm->endInsertRows();
                    }
                }
            }
        } else {
            children    = nc;
        }
        
        Image ni        = cdb::icon(folder);
        if(ni!=icoImg){
            icoImg      = ni;
            icon        = cdb::qIcon(ni);
            if(icon.isNull())
                icon    = si_folder();
            if(chgSignal & CHG_DATA){
                QModelIndex i   = ftm -> index(folder);
                ftm -> dataChanged(i,i);
            }
        }
    }
    
    int             row(Folder f) const
    {
        auto i = children.find(f);
        if(i != children.end())
            return (int)(i-children.begin());
        return -1;
    }
    
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Folder   FolderExplorerBase::TreeModel::folder(const QModelIndex&idx) const
{
    return idx.isValid() ? Folder{reinterpret_cast<uint64_t>(idx.internalPointer())} : cdb::top_folder();
}

FolderExplorerBase::TreeModel::TreeModel(FolderExplorerBase* parent) : QAbstractItemModel(parent), m_fx(parent)
{
}

FolderExplorerBase::TreeModel::~TreeModel()
{
    for(auto& i : m_nodes)
        delete i.second;
    m_nodes.clear();
}

int         FolderExplorerBase::TreeModel::columnCount(const QModelIndex&) const 
{
    return 1;
}

QVariant    FolderExplorerBase::TreeModel::data(const QModelIndex& idx, int role) const 
{
    const Node* n   = node(idx);
    
    switch(role){
    case Qt::DisplayRole:
        if(n && n->folder)
            return cdb::label(n->folder);
        break;
    case Qt::DecorationRole:
        if(n)
            return QVariant::fromValue(n->icon);
        break;
    }
    return QVariant();
}

QVariant    FolderExplorerBase::TreeModel::headerData(int,Qt::Orientation,int) const 
{
    return QVariant();
}


QModelIndex FolderExplorerBase::TreeModel::index(Folder f) const
{
    const Node*n    = node(f);
    if(!n)
        return QModelIndex();
    const Node*p    = node(n->parent);
    if(!p)
        return QModelIndex();
    return createIndex((int) p->children.first_index_of(f), 0, (void*) f.id);
    
}

QModelIndex FolderExplorerBase::TreeModel::index(int row, int column, const QModelIndex&par) const 
{
    Folder      f   = par.isValid() ? folder(par) : cdb::top_folder();
    const Node* n   = node(f);
    if(!n)
        return QModelIndex();
    if((row < 0) || (row>= (int) n->children.size()))
        return QModelIndex();
    return createIndex(row, column, (void*) n->children[row].id);
    
}

FolderExplorerBase::TreeModel::Node*          FolderExplorerBase::TreeModel::node(Folder f)
{
    Node* n         = m_nodes.get(f,nullptr);
    if(f && !n){
        n   = new Node(this, f);
        m_nodes[f]  = n;
        n -> refresh(0);
    }
    return n;
}

const FolderExplorerBase::TreeModel::Node*    FolderExplorerBase::TreeModel::node(Folder f) const
{
    return const_cast<TreeModel*>(this)->node(f);
}

FolderExplorerBase::TreeModel::Node*          FolderExplorerBase::TreeModel::node(const QModelIndex& idx)
{
    return node(folder(idx));
}

const FolderExplorerBase::TreeModel::Node*    FolderExplorerBase::TreeModel::node(const QModelIndex& idx) const
{
    return node(folder(idx));
}


QModelIndex FolderExplorerBase::TreeModel::parent(const QModelIndex& idx) const
{
    Folder  f   = folder(idx);
    if(!f)
        return QModelIndex();
    const Node* n     = node(f);
    if(!n || !n->parent)
        return QModelIndex();
    const Node* p    = node(n->parent);
    if(!p || !p->parent)
        return QModelIndex();
    const Node* gp    = node(p->parent);
    if(!gp)
        return QModelIndex();
    int row = gp->row(n->parent);
    return createIndex(row, idx.column(), (void*) n->parent.id);
}

void        FolderExplorerBase::TreeModel::refresh()
{
    //  TODO
}

int         FolderExplorerBase::TreeModel::rowCount(const QModelIndex&idx) const 
{
    const Node* n   = node(idx);
    if(!n)
        return 0;
    
    return (int) n -> children.size();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FolderExplorerBase::TreeView::TreeView(TreeModel*ftm, QWidget*parent) : QTreeView(parent), m_model(ftm)
{
    setModel(ftm);
    setHeaderHidden(true);
    setSelectionBehavior(SelectRows);
    setSelectionMode(SingleSelection);
    setEditTriggers(NoEditTriggers);
    connect(this, &QTreeView::activated, this, &FolderExplorerBase::TreeView::cmdActivated);
}

FolderExplorerBase::TreeView::~TreeView()
{

}

void    FolderExplorerBase::TreeView::cmdActivated(const QModelIndex& idx)
{
    Folder  f   = m_model -> folder(idx);
    if(f)
        emit activatedFolder(f);
}

Folder  FolderExplorerBase::TreeView::currentFolder() const
{
    return m_model -> folder(currentIndex());
}

void    FolderExplorerBase::TreeView::setCurrentFolder(Folder f)
{
    setCurrentIndex(m_model -> index(f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "moc_FolderExplorerBase.cpp"
#include "moc_FolderExplorerImpl.cpp"
