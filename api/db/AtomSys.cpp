////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#include "AtomSys.hpp"

#include "NKI.hpp"
#include "Root.hpp"
#include "Workspace.hpp"
#include "Graph.hpp"
#include "Image.hpp"
#include "Tag.hpp"

#include "util/Logging.hpp"
#include "util/SqlQuery.hpp"
#include "util/Utilities.hpp"

#include <QSqlError>

namespace cdb {

    QString             abbreviation(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT abbr FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    namespace {
        Vector<Atom>        all_atoms_sorted()
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Atoms ORDER BY k");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Atom>        all_atoms_unsorted()
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Atoms");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }
    }
    

    Vector<Atom>        all_atoms(Sorted sorted)
    {
        return sorted ? all_atoms_sorted() : all_atoms_unsorted();
    }
    
    namespace {
        Vector<Atom>    all_atoms_sorted(Class c)
        {
            Vector<Atom>    ret;
                    // I think this SQL is right.....
            static thread_local SqlQuery s(wksp::cache(), "SELECT atom FROM AClasses INNER JOIN Classes ON AClasses.class=Classes.id "
                                "WHERE class=? ORDER BY Classes.k");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Atom>    all_atoms_unsorted(Class c)
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT atom FROM AClasses WHERE class=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }
    }
    
    Vector<Atom>        all_atoms(Class c,Sorted sorted)
    {
        return sorted ? all_atoms_sorted(c) : all_atoms_unsorted(c);
    }
    
    namespace {
        Vector<Atom>    all_atoms_sorted(Tag t)
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT atom FROM ATags INNER JOIN Tags ON ATags.tag=Tags.id "
                            "WHERE tag=? ORDER BY Tags.k");
            auto s_af = s.af();
            s.bind(0, t.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Atom>    all_atoms_unsorted(Tag t)
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT atom FROM ATags WHERE tag=?");
            auto s_af = s.af();
            s.bind(0, t.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }
    }
    
    Vector<Atom>        all_atoms(Tag t,Sorted sorted)
    {
        return sorted ? all_atoms_sorted(t) : all_atoms_unsorted(t);
    }
    
    size_t              all_atoms_count()
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM Atoms");
        auto s_af = s.af();
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    size_t              all_atoms_count(Class c)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM AClasses WHERE class=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    size_t              all_atoms_count(Tag t)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM ATags WHERE tag=?");
        auto s_af = s.af();
        s.bind(0, t.id);
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    Vector<ClassPair>   all_class_dependencies()
    {
        Vector<ClassPair>   ret;
        static thread_local SqlQuery    s(wksp::cache(), "SELECT id,base FROM CDepends WHERE hops=0");
        auto s_af = s.af();
        if(s.exec()){
            while(s.next())
                ret << ClassPair{{s.valueU64(0)}, {s.valueU64(1)}};
        }
        return ret;
    }

    namespace {
        Vector<Class>   all_classes_sorted()
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM Classes ORDER BY name");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>   all_classes_unsorted()
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM Classes");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
    }

    Vector<Class>       all_classes(Sorted sorted)
    {
        return sorted ? all_classes_sorted() : all_classes_unsorted();
    }

    
    size_t              all_classes_count()
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM Classes");
        auto s_af = s.af();
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    namespace {
        Vector<Field>       all_fields_sorted()
        {
            Vector<Field>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Fields ORDER BY k");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Field{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Field>       all_fields_unsorted()
        {
            Vector<Field>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Fields");
            auto s_af = s.af();
            if(s.exec()){
                while(s.next())
                    ret << Field{s.valueU64(0)};
            }
            return ret;
        }
    }
    

    Vector<Field>       all_fields(Sorted sorted)
    {
        return sorted ? all_fields_sorted() : all_fields_unsorted();
    }
    
    size_t              all_fields_count()
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM Fields");
        auto s_af = s.af();
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    
    Atom                atom(uint64_t i)
    {
        return exists_atom(i) ? Atom{i} : Atom();
    }
    
    Atom                atom(const QString& k)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Atoms WHERE k=? LIMIT 1");
        auto s_af = s.af();
        s.bind(0, k);
        if(s.exec() && s.next())
            return Atom{s.valueU64(0)};
        return Atom();
    }

    namespace {
        Vector<Atom>    atoms_sorted(Document doc)
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Atoms WHERE doc=? ORDER BY k");
            auto s_af = s.af();
            s.bind(0, doc.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Atom>    atoms_unsorted(Document doc)
        {
            Vector<Atom>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Atoms WHERE doc=?");
            auto s_af = s.af();
            s.bind(0, doc.id);
            if(s.exec()){
                while(s.next())
                    ret << Atom{s.valueU64(0)};
            }
            return ret;
        }
    }
    
    Vector<Atom>        atoms(Document doc, Sorted sorted)
    {
        return sorted ? atoms_sorted(doc) : atoms_unsorted(doc);
    }
    
    size_t              atoms_count(Document doc)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM Atoms WHERE doc=?");
        auto s_af = s.af();
        s.bind(0, doc.id);
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    QString             brief(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT brief FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    QString             brief(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT brief FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    QString             brief(Field f)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT brief FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    Class               class_(Field f)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT class FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return Class(s.valueU64(0));
        return Class{};
    }

    Class               class_(uint64_t i)
    {
        return exists_class(i) ? Class{i} : Class();
    }
    
    Class               class_(const QString& k)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Classes WHERE k=?");
        auto s_af = s.af();
        s.bind(0, k);
        if(s.exec() && s.next())
            return Class{s.valueU64(0)};
        return Class{};
    }

    ClassFile::Shared        class_doc(Fragment f, bool fAllowEmpty)
    {
        if(!f)
            return ClassFile::Shared();
            
        Vector<char>    ch   = chars(f);
        if(ch.empty())
            return fAllowEmpty ? std::make_shared<ClassFile>() : ClassFile::Shared();
            
        ClassFile::Shared  td = std::make_shared<ClassFile>();
        if(!td->load(ch, path(f).toStdString())){
            yError() << "Unable to read " << path(f);
            return ClassFile::Shared();
        }
        td -> set_file(path(f));
        return td;
    }
    

    

    
    namespace {
        //Vector<Class>       classes_sorted(Atom a)
        
        Vector<Class>       classes_unsorted(Atom a)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT class FROM AClasses WHERE atom=?");
            auto s_af = s.af();
            s.bind(0, a.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
    }
    
    
    Vector<Class>       classes(Atom a, Sorted sorted)
    {
        return classes_unsorted(a); // TODO: Sorted
    }
    
    namespace {
        Vector<Class>   classes_any(Field f)
        {
            Vector<Class> ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT class FROM CFields WHERE field=?");
            auto s_af = s.af();
            s.bind(0, f.id);
            if(s.exec()){
                while(s.next())
                    ret << Class(s.valueU64(0));
            }
            return ret;
        }
        
        Vector<Class>   classes_direct(Field f)
        {
            Vector<Class> ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT class FROM CFields WHERE field=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, f.id);
            if(s.exec()){
                while(s.next())
                    ret << Class(s.valueU64(0));
            }
            return ret;
        }
        
        Vector<Class>   classes_indirect(Field f)
        {
            Vector<Class> ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT class FROM CFields WHERE field=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, f.id);
            if(s.exec()){
                while(s.next())
                    ret << Class(s.valueU64(0));
            }
            return ret;
        }
    }
    
    Vector<Class>       classes(Field f, Linkage l)
    {
        switch(l){
        case Linkage::Any:
            return classes_any(f);
        case Linkage::Direct:
            return classes_direct(f);
        case Linkage::Indirect:
            return classes_indirect(f);
        case Linkage::None:
        default:
            return Vector<Class>();
        }
    }

    Vector<Class>       classes(const StringSet& sset)
    {
        Vector<Class>   ret;
        for(String s : sset)
            ret << db_class(s.qString());
        return ret;
    }
    
    Vector<Class>       classes(const QStringSet& sset)
    {
        Vector<Class>   ret;
        for(QString s : sset)
            ret << db_class(s);
        return ret;
    }
    
    size_t              classes_count(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM AClasses WHERE atom=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }    
    
    Atom                db_atom(Document d, bool* wasCreated)
    {
        return db_atom(d, QString(), wasCreated);
    }
    
    Atom                db_atom(Document d, const QString&ck, bool* wasCreated)
    {
        if(wasCreated)
            *wasCreated = false;

        QString     dk = strip_extension(key(d));
        if(dk.isEmpty()){
            yError() << "Cannot create to an empty key!";
            return Atom();
        }

        if(!ck.isEmpty())
            dk      += "#" + ck;
        
        static thread_local SqlQuery    i(wksp::cache(), "INSERT OR FAIL INTO Atoms (k,doc) VALUES (?,?)");
        static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM Atoms WHERE k=?");
        auto i_af   = i.af();
        auto s_af   = s.af();
        i.bind(0, dk);
        i.bind(1, d.id);
        
        if(i.exec(false)){
            if(wasCreated)
                *wasCreated = true;
            return Atom{i.lastInsertIdU64()};
        } else {
            s.bind(0, dk);
            if(s.exec() && s.next())
                return Atom{s.valueU64(0)};
            yError() << "Unable to get the atom from the database: " << dk;
            return Atom();
        }
    }
    
    Class               db_class(const QString&k, bool *wasCreated)
    {
        return db_class(db_document(classes_folder(), k+".cls"), wasCreated);
    }
    
    Class               db_class(Document doc, bool *wasCreated)
    {
        if(wasCreated)
            *wasCreated = false;
        if(folder(doc) != classes_folder())
            return Class{};
        QString k   = base_key(doc);
        if(k.isEmpty())
            return Class();
        
        static thread_local SqlQuery    i(wksp::cache(), "INSERT OR FAIL INTO Classes (k,id) VALUES (?,?)");
        auto i_lk   = i.af();

        i.bind(0, k);
        i.bind(1, doc.id);
        if(i.exec(false)){
            if(wasCreated)
                *wasCreated = true;
            return Class{doc.id};
        } else if(exists_class(doc.id)){
            return Class{doc.id};
        } else {
            yError() << "Unable to get the class from the database: " << k;
            return Class();
        }
    }

    Field               db_field(Class c, const QString&k, bool *wasCreated)
    {
        if(wasCreated)
            *wasCreated = false;
        if(!c)
            return Field{};
        if(k.isEmpty())
            return Field{};
            
        static thread_local SqlQuery i(wksp::cache(), "INSERT INTO Fields (class, k) VALUES (?,?)");
        static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Fields WHERE class=? AND k=?");
        auto i_af = i.af();
        auto s_af = s.af();
        i.bind(0, c.id);
        i.bind(1, k);
        if(i.exec()){
            if(wasCreated)
                *wasCreated = true;
            return Field(i.lastInsertIdU64());
        } else {
            s.bind(0, c.id);
            s.bind(1, k);
            if(s.exec() && s.next())
                return Field(s.valueU64(0));
            
            yInfo() << "Unable to create field ID!";
            return Field();
        }
    }
    
    Graph               dep_graph(Class c)
    {   
        static thread_local SqlQuery    s(wksp::cache(), "SELECT deps FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return Graph{ s.valueU64(0)};
        return Graph{};
    }

    namespace {
        //Vector<Class>   dependents_sorted(Class, Linkage);    // TODO (INNER JOIN)
        Vector<Class>   dependents_unsorted_any(Class c)
        {
            Vector<Class> ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM CDepends WHERE base=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{ s.valueU64(0) };
            }
            return ret;
        }

        Vector<Class>   dependents_unsorted_direct(Class c)
        {
            Vector<Class> ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM CDepends WHERE base=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{ s.valueU64(0) };
            }
            return ret;
        }
        
        Vector<Class>   dependents_unsorted_indirect(Class c)
        {
            Vector<Class> ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT id FROM CDepends WHERE base=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{ s.valueU64(0) };
            }
            return ret;
        }

        Vector<Class>   dependents_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return dependents_unsorted_any(c);
            case Linkage::Direct:
                return dependents_unsorted_direct(c);
            case Linkage::Indirect:
                return dependents_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Class>();
            }
        }
    }

    Vector<Class>       dependents(Class c, Linkage l, Sorted sorted)
    {
        return dependents_unsorted(c, l);   // TODO INNER JOIN
    }

    Vector<Class>       dependents(Class c, Sorted sorted)
    {
        return dependents_unsorted_any(c);  // TODO
    }

    Vector<Class>       dependents(Class c, Sorted::Value sorted)
    {
        return dependents_unsorted_any(c);  // TODO
    }

    

    Document            document(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT doc FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return Document{s.valueU64(0)};
        return Document();
    }
    
    Document            document(Class c)
    {
        return exists(c) ? Document{c.id}  : Document{};
    }
    
    Document                document(Field f)
    {
        return document(class_(f));
    }

    bool                edge(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT edge FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return s.valueAs<bool>(0);
        return false;
    }

    bool                exists(Atom a)
    {
        return exists_atom(a.id);
    }
    
    bool                exists(Class c)
    {
        return exists_class(c.id);
    }
    
    
    bool                exists(Field f)
    {
        return exists_field(f.id);
    }
    

   
    bool                exists_atom(uint64_t i)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT 1 FROM Atoms WHERE id=? LIMIT 1");
        auto s_lk   = s.af();
        s.bind(0, i);
        if(s.exec() && s.next())
            return s.valueAs<bool>(0);
        return false;
    }

    bool                exists_class(uint64_t i)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT 1 FROM Classes WHERE id=? LIMIT 1");
        auto s_lk   = s.af();
        s.bind(0, i);
        if(s.exec() && s.next())
            return s.valueAs<bool>(0);
        return false;
    }


    bool                exists_field(uint64_t i)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT 1 FROM Fields WHERE id=? LIMIT 1");
        auto s_lk   = s.af();
        s.bind(0, i);
        if(s.exec() && s.next())
            return s.valueAs<bool>(0);
        return false;
    }

    Field               field(uint64_t i)
    {
        return exists_field(i) ? Field{i} : Field{};
    }
    
    Field               field(Class c, const QString&k)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT id FROM Fields WHERE cls=? AND k=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        s.bind(1, k);
        if(s.exec() && s.next())
            return Field{s.valueU64(0)};
        return Field{};
    }
    

    namespace {
        
        #if 0
            //  TODO ... Requires INNER JOIN
        Vector<Field>   fields_sorted_any(Class c)
        {
        }
        
        Vector<Field>   fields_sorted_direct(Class c)
        {
        }

        Vector<Field>   fields_sorted_indirect(Class c)
        {
        }
    
        Vector<Field>   fields_sorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return fields_sorted_any(c);
            case Linkage::Direct:
                return fields_sorted_direct(c);
            case Linkage::Indirect:
                return fields_sorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Field>();
            }
        }
        #endif
        
        Vector<Field>   fields_unsorted_any(Class c)
        {
            Vector<Field>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT field FROM CFields WHERE class=?");
            auto s_af   = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Field(s.valueU64(0));
            }
            return ret;
        }
        
        Vector<Field>   fields_unsorted_direct(Class c)
        {
            Vector<Field>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT field FROM CFields WHERE class=? AND hops=0");
            auto s_af   = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Field(s.valueU64(0));
            }
            return ret;
        }

        Vector<Field>   fields_unsorted_indirect(Class c)
        {
            Vector<Field>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT field FROM CFields WHERE class=? AND hops!=0");
            auto s_af   = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Field(s.valueU64(0));
            }
            return ret;
        }

        Vector<Field>   fields_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return fields_unsorted_any(c);
            case Linkage::Direct:
                return fields_unsorted_direct(c);
            case Linkage::Indirect:
                return fields_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Field>();
            }
        }
    }


    Vector<Field>       fields(Class c, Linkage l, Sorted sorted)
    {
        return fields_unsorted(c,l);
        // TODO (get inner join working to perform sorted queries)
        //return sorted ? fields_sorted(c,l) : fields_unsorted(c,l);
    }
    
    namespace {
        size_t          fields_count_any(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CFields WHERE class=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          fields_count_direct(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CFields WHERE class=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          fields_count_indirect(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CFields WHERE class=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }
    }
    
    
    size_t              fields_count(Class c, Linkage l)
    {
        switch(l){
        case Linkage::Any:
            return fields_count_any(c);
        case Linkage::Direct:
            return fields_count_direct(c);
        case Linkage::Indirect:
            return fields_count_indirect(c);
        case Linkage::None:
        default:
            return 0;
        }
    }
    
   
    Image               icon(Atom a) 
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT icon FROM Atoms WHERE id=? LIMIT 1");
        auto s_af   = s.af();
        s.bind(0,a.id);
        if(s.exec() && s.next())
            return Image{s.valueU64(0)};
        return Image();
    }
    
    Image               icon(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT icon FROM Classes WHERE id=? LIMIT 1");
        auto s_af   = s.af();
        s.bind(0,c.id);
        if(s.exec() && s.next())
            return Image{s.valueU64(0)};
        return Image();
    }
    
    
    Image               icon(Field f)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT icon FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return Image(s.valueU64(0));
        return Image{};
    }
    
    Atom::Info          info(Atom a)
    {
        Atom::Info    ret;
        
        static thread_local SqlQuery s(wksp::cache(), "SELECT k, abbr, doc, brief FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next()){
            ret.key         = s.valueString(0);
            ret.abbr        = s.valueString(1);
            ret.doc         = Document(s.valueU64(2));
            ret.brief       = s.valueString(3);
        }
        return ret;
    }
    
    Class::Info         info(Class c, bool autoKey)
    {
        Class::Info    ret;
        static thread_local SqlQuery    s(wksp::cache(), "SELECT k, edge, name, plural, brief, icon, deps FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next()){
            ret.key         = s.valueString(0);
            ret.doc         = Document(c.id);
            ret.edge        = s.valueAs<bool>(1);
            ret.name        = s.valueString(2);
            ret.plural      = s.valueString(3);
            ret.brief       = s.valueString(4);
            ret.icon        = Image(s.valueU64(5));
            ret.deps        = Graph(s.valueU64(6));
            if(autoKey && ret.name.isEmpty())
                ret.name    = ret.key;
        }
        return ret;
    }
    

    
    Field::Info         info(Field f, bool autoKey)
    {
        Field::Info        ret;
        static thread_local SqlQuery s(wksp::cache(), "SELECT k, cls, name, pkey, plural, brief FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next()){
            ret.key     = s.valueString(0);
            ret.class_  = Class(s.valueU64(1));
            ret.name    = s.valueString(2);
            ret.pkey    = s.valueString(3);
            ret.plural  = s.valueString(4);
            ret.brief   = s.valueString(5);
            if(autoKey && ret.name.isEmpty())
                ret.name    = ret.key;
        }
        return ret;
    }
    
    bool                is(Atom a, Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT 1 FROM AClasses WHERE atom=? AND class=?");
        auto s_af       = s.af();
        s.bind(0, a.id);
        s.bind(1, c.id);
        return s.exec() && s.next();
    }
    
    bool                is(Atom a, std::initializer_list<Class>cs)
    {
        for(Class c : cs)
            if(!is(a, c))
                return false;
        return true;
    }
    
 

    QString             key(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT k FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    QString             key(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT k FROM Classes WHERE id=? LIMIT 1");
        auto s_af       = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    
    QString             key(Field f)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT k FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    
    QString             label(Atom a)
    {
        return key(a);  // hard wire for now
    }
    
    QString             label(Class c)
    {
        QString     n   = name(c);
        return n.isEmpty() ? key(c) : n;
    }

    QString             label(Field f)
    {
        QString     n = name(f);
        return n.isEmpty() ? key(f) : n;
    }
    
    //Leaf                    leaf(Atom

    Class                       make_class(const QString&k, const Root* rt)
    {
        if(!rt)
            rt      = wksp::root_first(DataRole::Classes);
        if(!rt){
            yError() << "No root specified to create the class in!";
            return Class{};
        }
        
        Document        doc = db_document(classes_folder(), k+".cls");
        bool            was = false;
        Class           c   = db_class(doc, &was);
        if(!was)
            return c;
        if(fragments_count(doc))
            return c;
        
        ClassFile::Shared    td  = write(c, rt);
        td -> name      = k;
        td -> plural    = k + 's';
        td -> save();
        return c;
    }
    

    ClassFile::Shared            merged(Class c, unsigned int opts)
    {
        ClassFile::Shared        ret = std::make_shared<ClassFile>();;
        for(auto& i : reads(c)){
            if(opts & IsUpdate)
                update(i.first);
            ret->merge(*(i.second), static_cast<bool>(opts&Override));
        }
        return ret;
    }
    


    QString             name(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT name FROM Classes WHERE id=? LIMIT 1");
        auto s_af       = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }

    
    QString             name(Field f)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT name FROM Fields WHERE id=? LIMIT 1");
        auto s_af       = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }

    
    NKI                 nki(Atom a, bool autoKey)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT name,icon,k FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next()){
            NKI  ret;
            ret.name    = s.valueString(0);
            ret.icon    = Image{s.valueU64(1)};
            ret.key     = s.valueString(2);
            if(autoKey && ret.name.isEmpty())
                ret.name    = ret.key;
            return ret;
        }
        return NKI();
    }
    
    NKI                 nki(Class c, bool autoKey)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT name,icon,k FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next()){
            NKI  ret;
            ret.name    = s.valueString(0);
            ret.icon    = Image{s.valueU64(1)};
            ret.key     = s.valueString(2);
            if(autoKey && ret.name.isEmpty())
                ret.name    = ret.key;
            return ret;
        }
        return NKI();
    }
    
    
    NKI                 nki(Field f, bool autoKey)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT name,icon,k FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next()){
            NKI  ret;
            ret.name    = s.valueString(0);
            ret.icon    = Image(s.valueU64(1)) ;
            ret.key     = s.valueString(2);
            if(autoKey && ret.name.isEmpty())
                ret.name    = ret.key;
            return ret;
        }
        return NKI{};
    }    

    
    Class               parent(Field f)
    {
        return class_(f);
    }
    
    

    
    QString             pkey(Field f)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT pkey FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    QString             plural(Class c)
    {
        static thread_local SqlQuery    s(wksp::cache(), "SELECT plural FROM Classes WHERE id=?");
        auto s_af = s.af();
        s.bind(0, c.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    
    QString             plural(Field f)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT plural FROM Fields WHERE id=?");
        auto s_af = s.af();
        s.bind(0, f.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }
    


    ClassFile::Shared        read(Class c, const Root*rt)
    {
        return class_doc(fragment(document(c), rt));
    }

    Vector<Class::FragDoc>    reads(Class c)
    {
        Vector<Class::FragDoc>  ret;
        for(Fragment f : fragments(document(c), DataRole::Classes)){
            ClassFile::Shared    p   = class_doc(f);
            if(p)
                ret << Class::FragDoc(f, p);
        }
        return ret;
    }

    Vector<Class::FragDoc>  reads(Class c, class Root*rt)
    {
        Vector<Class::FragDoc>  ret;
        for(Fragment f : fragments(document(c), rt)){
            ClassFile::Shared    p   = class_doc(f);
            if(p)
                ret << Class::FragDoc(f, p);
        }
        return ret;
    }
    




    
    namespace {
        //Vector<Class>       reverses_sorted(Class, Linkage l);  // TODO once the INNER JOINS are good

        Vector<Class>       reverses_unsorted_any(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT rev FROM CReverses WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Class>       reverses_unsorted_direct(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT rev FROM CReverses WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>       reverses_unsorted_indirect(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT rev FROM CReverses WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Class>       reverses_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return reverses_unsorted_any(c);
            case Linkage::Direct:   
                return reverses_unsorted_direct(c);
            case Linkage::Indirect:
                return reverses_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Class>();
            }
        }
    }
    
    Vector<Class>       reverses(Class c, Linkage l, Sorted sorted)
    {
        //  TODO (sorted, INNER JOIN)
        return reverses_unsorted(c,l);
    }

    Vector<Class>       reverses(Class c, Sorted sorted)
    {
        return reverses_unsorted_any(c);    // TODO
    }
    
    Vector<Class>       reverses(Class c, Sorted::Value sorted)
    {
        return reverses_unsorted_any(c);    // TODO
    }
    


    namespace {
        //Vector<Class>   sources_sorted(Class, Linkage);     // TODO INNER JOIN
        Vector<Class>       sources_unsorted_any(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT src FROM CSources WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>       sources_unsorted_direct(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT src FROM CSources WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>       sources_unsorted_indirect(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT src FROM CSources WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Class>       sources_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return sources_unsorted_any(c);
            case Linkage::Direct:
                return sources_unsorted_direct(c);
            case Linkage::Indirect:
                return sources_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Class>();
            }
        }
        
    }
    
    Vector<Class>       sources(Class c, Linkage l, Sorted sorted)
    {
        return sources_unsorted(c,l);   // TODO 
    }
    
    Vector<Class>       sources(Class c, Sorted sorted)
    {
        return sources_unsorted_any(c); // TODO
    }

    Vector<Class>       sources(Class c, Sorted::Value sorted)
    {
        return sources_unsorted_any(c); // TODO
    }

    namespace {
        size_t          sources_count_any(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CSources WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          sources_count_direct(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CSources WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          sources_count_indirect(Class c)
        {
            static thread_local SqlQuery    s(wksp::cache(), "SELECT COUNT(1) FROM CSources WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }
    }

    size_t              sources_count(Class c, Linkage l)
    {
        switch(l){
        case Linkage::Any:
            return sources_count_any(c);
        case Linkage::Direct:
            return sources_count_direct(c);
        case Linkage::Indirect:
            return sources_count_indirect(c);
        case Linkage::None:
        default:
            return 0;
        }
    }
    
    Vector<Class>           targets(Class, Linkage l,Sorted sorted);
    Vector<Class>           targets(Class, Sorted sorted);
    Vector<Class>           targets(Class, Sorted::Value sorted);
    size_t                  targets_count(Class, Linkage l);
    


    namespace {
        // Vector<Tag>     tags_sorted(Atom a);     // TODO (INNER JOIN)
        Vector<Tag>     tags_unsorted(Atom a)
        {
            Vector<Tag>    ret;
            static thread_local SqlQuery s(wksp::cache(), "SELECT tag FROM ATags WHERE atom=?");
            auto s_af = s.af();
            s.bind(0, a.id);
            if(s.exec()){
                while(s.next())
                    ret << Tag{s.valueU64(0)};
            }
            return ret;
        }
    }
    
    
    Vector<Tag>         tags(Atom a, Sorted sorted)
    {
        return tags_unsorted(a);
        //  TODO sorted....
    }

    size_t              tags_count(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM ATags WHERE atom=?");
        auto s_af   = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return (size_t) s.valueU64(0);
        return 0;
    }
    
    bool                tagged(Atom a, Tag t)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT 1 FROM ATags WHERE atom=? AND tag=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        s.bind(1, t.id);
        return s.exec() && s.next();
    }
    
    bool                tagged(Atom a, std::initializer_list<Tag> ts)
    {
        for(Tag t : ts)
            if(!tagged(a, t))
                return false;
        return true;
    }



    


    namespace {
        //Vector<Class>   targets_sorted(Class c, Linkage l);     // TODO (INNER JOIN)
        
        Vector<Class>   targets_unsorted_any(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT tgt FROM CTargets WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Class>   targets_unsorted_direct(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT tgt FROM CTargets WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>   targets_unsorted_indirect(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT tgt FROM CTargets WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }

        Vector<Class>   targets_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return targets_unsorted_any(c);
            case Linkage::Direct:
                return targets_unsorted_direct(c);
            case Linkage::Indirect:
                return targets_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Class>();
            }
        }
        
    }

    Vector<Class>       targets(Class c, Linkage l, Sorted sorted)
    {
        return targets_unsorted(c,l);   // TODO 
    }
    
    Vector<Class>       targets(Class c, Sorted sorted)
    {
        return targets_unsorted_any(c); // TODO (INNER JOIN)
    }

    Vector<Class>       targets(Class c, Sorted::Value sorted)
    {
        return targets_unsorted_any(c); // TODO (INNER JOIN)
    }

    namespace {
        size_t          targets_count_any(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CTargets WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          targets_count_direct(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CTargets WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          targets_count_indirect(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CTargets WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }
    }
    
    size_t              targets_count(Class c, Linkage l)
    {
        switch(l){
        case Linkage::Any:
            return targets_count_any(c);
        case Linkage::Direct:
            return targets_count_direct(c);
        case Linkage::Indirect:
            return targets_count_indirect(c);
        case Linkage::None:
        default:
            return 0;
        }
    }
    

    QString             title(Atom a)
    {
        static thread_local SqlQuery s(wksp::cache(), "SELECT title FROM Atoms WHERE id=?");
        auto s_af = s.af();
        s.bind(0, a.id);
        if(s.exec() && s.next())
            return s.valueString(0);
        return QString();
    }

    namespace {
        //Vector<Class>   uses_sorted(Class c, Linkage l);    // TODO INNER JOIN
        Vector<Class>   uses_unsorted_any(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT base FROM CDepends WHERE id=?");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>   uses_unsorted_direct(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT base FROM CDepends WHERE id=? AND hops=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>   uses_unsorted_indirect(Class c)
        {
            Vector<Class>   ret;
            static thread_local SqlQuery    s(wksp::cache(), "SELECT base FROM CDepends WHERE id=? AND hops!=0");
            auto s_af = s.af();
            s.bind(0, c.id);
            if(s.exec()){
                while(s.next())
                    ret << Class{s.valueU64(0)};
            }
            return ret;
        }
        
        Vector<Class>   uses_unsorted(Class c, Linkage l)
        {
            switch(l){
            case Linkage::Any:
                return uses_unsorted_any(c);
            case Linkage::Direct:
                return uses_unsorted_direct(c);
            case Linkage::Indirect:
                return uses_unsorted_indirect(c);
            case Linkage::None:
            default:
                return Vector<Class>();
            }
        }
    }
    
    Vector<Class>       uses(Class c, Linkage l, Sorted sorted)
    {
        return uses_unsorted(c, l);
        //  TODO (FIX ON INNER JOIN)
    }

    Vector<Class>       uses(Class c, Sorted sorted)
    {
        //  TODO (once sorted is a thing)
        return uses_unsorted_any(c);
    }

    Vector<Class>       uses(Class c, Sorted::Value sorted)
    {
        //  TODO (once sorted is a thing)
        return uses_unsorted_any(c);
    }

    namespace {
        size_t          uses_count_any(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CDepends WHERE id=?");
            auto s_af   = s.af();
            s.bind(0,c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          uses_count_direct(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CDepends WHERE id=? AND hops=0");
            auto s_af   = s.af();
            s.bind(0,c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }

        size_t          uses_count_indirect(Class c)
        {
            static thread_local SqlQuery s(wksp::cache(), "SELECT COUNT(1) FROM CDepends WHERE id=? AND hops!=0");
            auto s_af   = s.af();
            s.bind(0,c.id);
            if(s.exec() && s.next())
                return (size_t) s.valueU64(0);
            return 0;
        }
    }

    size_t              uses_count(Class c, Linkage l)
    {
        switch(l){
        case Linkage::Any:
            return uses_count_any(c);
        case Linkage::Direct:
            return uses_count_direct(c);
        case Linkage::Indirect:
            return uses_count_indirect(c);
        case Linkage::None:
        default:
            return 0;
        }
    }

    
    ClassFile::Shared        write(Class c, const Root* rt)
    {
        Document    d   = document(c);
        if(!d)
            return ClassFile::Shared();
        Fragment    f   = fragment(d);
        if(f)
            return class_doc(f, true);
        Folder      fo  = folder(d);
        if((fo != cdb::top_folder()) && !exists(fo, rt))
            rt -> make_path(key(fo));
        ClassFile::Shared ptr  = std::make_shared<ClassFile>();
        ptr -> set_file( rt -> resolve(key(d)));
        return ptr;
    }


}
