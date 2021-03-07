////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#include "yCommon.hpp"
#include "yUpdater.hpp"

#include <db/Tag.hpp>
#include <db/ShareDir.hpp>
#include <db/Workspace.hpp>
#include <srv/Importer.hpp>
#include <srv/Page.hpp>
#include <srv/Scanner.hpp>
#include <util/FileUtils.hpp>
#include <util/Guarded.hpp>
#include <util/Utilities.hpp>

//#include "yImporter.hpp"
//#include "yScanner.hpp"
//#include "yUpdater.hpp"

//#include "db/Cache.hpp"
//#include "util/FileUtils.hpp"
//#include "util/Logging.hpp"
//#include "db/ShareDir.hpp"
//#include "util/SqlQuery.hpp"
//#include "util/Compare.hpp"
//#include "util/Guarded.hpp"
//#include "util/Map.hpp"
//#include "util/Utilities.hpp"
//#include "util/Vector.hpp"


using namespace cdb;


/*
    This section is about maintaining the data graph, in memory, and it needs to remain lightweight.
*/
    
uAtom&      uget(Atom a)
{
    static Vector<uAtom*>       data(65536, nullptr);
    data.resize_if_under(a.id+1,2048,nullptr);
    uAtom*& p = data[a.id];
    if(!p)
        p   = new uAtom(a);
    return *p;
}

uClass&            uget(Class c)
{
    static Vector<uClass*>      data(512, nullptr);
    data.resize_if_under(c.id+1, 64, nullptr);
    uClass*& p = data[c.id];
    if(!p)
        p   = new uClass(c);
    return *p;
}

uField&             uget(Field f)
{
    static Vector<uField*>      data(2048, nullptr);
    data.resize_if_under(f.id+1,256,nullptr);
    uField*& p = data[f.id];
    if(!p)
        p       = new uField(f);
    return *p;
}

uLeaf&              uget(Leaf l)
{
    static Vector<uLeaf*>       data(8192, nullptr);
    data.resize_if_under(l.id+1, 1024, nullptr);
    uLeaf*& p = data[l.id];
    if(!p)
        p   = new uLeaf(l);
    return *p;
}
    
    

namespace {

    /*
        We've got some common data here.... cache it!
    */

    std::atomic<bool>       gHasBackground{false};
    Guarded<TypedBytes>     gBackground;
    Guarded<TypedBytes>     gIndex;
    Guarded<QByteArray>     gFooter;
    Guarded<QByteArray>     gCss;
    Guarded<QByteArray>     gSummary;
    Guarded<QString>        gTextColor, gBkColor;

    void    update_css();

    void    update_background()
    {
        TypedBytes  n   = TypedBytes(DataRole::Image, {".background.jpg", ".backgorund.png", ".background.svg"});
        gBackground = n;
        if(gHasBackground != !n.content.isEmpty()){
            gHasBackground  = !n.content.isEmpty();
            update_css();
        }
    }

    void    update_css()
    {
        QByteArray      css;
        if(wksp::has_init()){
            for(Fragment f : cdb::fragments(".css", DataRole::Style))
                css += cdb::bytes(f);
        }
        
        QByteArray  newTxtColor     = "black";
        QByteArray  newBkColor      = "white";
        
        Map<QByteArray, QByteArray> vars;
        bool    inBody  = false;
        for(QByteArray l : css.split('\n')){
            if(inBody){
                if(l.startsWith('}')){
                    inBody  = false;
                    continue;
                }
                int i  = l.indexOf(':');
                if(i<=0)
                    continue;
                QByteArray  k   = l.mid(0,i).simplified().toLower();
                QByteArray  v   = l.mid(i+1).trimmed();
                if(k == "color")
                    newTxtColor      = v;
                if(k == "background-color")
                    newBkColor      = v;
                vars[k]     = v;
            } else if(l.toLower().startsWith("body")) {
                inBody  = true;
            }
        }
        gTextColor  = utf8(newTxtColor);
        gBkColor    = utf8(newBkColor);
        if(!vars.empty()){
            css += "\n/*\n    Variables auto-picked from existing CSS.\n*/\n";
            css += ":root {\n";
            for(auto& j : vars)
                css += "    --" + j.first + ": " + j.second + '\n';
            css += "}\n\n";
        }
        if(gHasBackground){
            css += "body {\n"
                    "    background-image: url(\"/background\");"
                    "}\n";
        }
            
        css        += shared_bytes("std/css");
        gCss        = css;
    }
    
    void    update_footer()
    {
        qbytearray_r    r;
        if(wksp::has_init())
            r               = cdb::bytes(".footer");
        if(!r.good || r.value.isEmpty())
            r.value         = shared_bytes("std/footer");
        gFooter             = r.value;
    }
    
    void    update_page()
    {
        Page::default_page(shared_bytes("std/page"));
    }
    
    void    update_index()
    {
        TypedBytes      ctr({".index.html", ".index.htm", ".index.md", ".index"}, ContentType::markdown);
        if(ctr.empty()){
            ctr.content             = shared_bytes("std/index");
            ctr.type                = ContentType::markdown;
        }
        gIndex      = ctr;
    }
    
    void    update_summary()
    {
        if(wksp::has_init())
            gSummary    = file_bytes(".summary");
    }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //  OTHER STUFF

    void    on_image_change(Fragment fragment)
    {
        update(db_image(document(fragment)));
    }
}

bool        has_background()
{
    return gHasBackground;
}

TypedBytes  cur_background()
{
    return gBackground;
}

TypedBytes  cur_index()
{
    return gIndex;
}

QByteArray  cur_footer()
{
    return gFooter;
}

QByteArray  cur_css()
{
    return gCss;
}

QByteArray  cur_summary()
{
    return gSummary;
}

QString     cur_text_color()
{
    return gTextColor;
}

QString     cur_back_color()
{
    return gBkColor;
}


void        updater_init()
{
    update_css();
    update_footer();
    update_page();
    update_index();

    Scanner::watch(shared_file("std/css"), update_css);
    Scanner::watch(shared_file("std/index"), update_index);
    Scanner::watch(shared_file("std/page"), update_page);
    Scanner::watch(shared_file("std/footer"), update_footer);

    on_change(all_set<Change>(), ".css", update_css);
    //on_change(all_set<Change>(), ".page", update_page);
    on_change(all_set<Change>(), ".footer", update_footer);
    on_change(all_set<Change>(), ".index", update_index);
    on_change(all_set<Change>(), ".index.md", update_index);
    on_change(all_set<Change>(), ".index.htm", update_index);
    on_change(all_set<Change>(), ".index.html", update_index);
    //on_change(all_set<Change>(), ".theme", update_css);
    on_change(all_set<Change>(), ".summary", update_summary);

    on_change(all_set<Change>(), ".background.jpg", update_background);
    on_change(all_set<Change>(), ".background.png", update_background);
    on_change(all_set<Change>(), ".background.svg", update_background);

    for(const char* z : { "bmp", "gif", "jpg", "png", "svg", "tif", "tiff" }){
        for(Document d : cdb::all_documents_suffix(z))
            cdb::update( cdb::db_image(d));
        on_change("*." + QString(z), on_image_change);
    }
    for(Document d : documents_by_suffix(cdb::tags_folder(), "tag"))
        cdb::update(cdb::db_tag(d), Update::Interior);

    init_class();
    init_leaf();

    for(Tag  t : all_tags())
        cdb::update(t, Update::Exterior);
    

    init_class_graphs();

    on_change("*.tag", [](Fragment frag){
        if(cdb::folder(frag) == cdb::tags_folder()){
            cdb::update(cdb::db_tag(frag), { Update::Exterior, Update::Interior });
        }
    });
}
