#include "Loggers.hpp"
#include "SqlQuery.hpp"
#include "Set.hpp"
#include "String.hpp"
#include "Vector.hpp"
#include "Utilities.hpp"

#include <QDebug>
#include <QSqlError>

namespace {
    struct QStringListAttachment : public SqlQuery::Attachment {
        QStringList&        retval;
        QStringListAttachment(QStringList& val) : retval(val)
        {
        }
        
        void    value(const QVariant& v) override 
        {
            retval << v.toString();
        };
        
    };

   struct StringSetAttachment : public SqlQuery::Attachment {
        StringSet&        retval;
        StringSetAttachment(StringSet& val) : retval(val)
        {
        }
        
        void    value(const QVariant& v) override 
        {
            retval << v.toString();
        };
        
    };

   struct StringVectorAttachment : public SqlQuery::Attachment {
        Vector<QString>&        retval;
        StringVectorAttachment(Vector<QString>& val) : retval(val)
        {
        }
        
        void    value(const QVariant& v) override 
        {
            retval << v.toString();
        };
        
    };
}

SqlQuery::SqlQuery(QSqlDatabase db, const QString& s ) : QSqlQuery(db)
{
    if(!s.isEmpty())
        prepare(s);
}

SqlQuery::SqlQuery(SqlQuery&&) = default;


SqlQuery::~SqlQuery()
{
    for(auto& i : m_attach){
        if(i.second)
            delete i.second;
    }
    m_attach.clear();
}

SqlQuery&   SqlQuery::operator=(SqlQuery&&) = default;

SqlQuery::AutoFinish  SqlQuery::af()
{
    return AutoFinish(this);
}

void        SqlQuery::attach(int n, Attachment*att)
{
    setAttach(n, att);
}

void        SqlQuery::attach(int n, QStringList& slist)
{
    setAttach(n, new QStringListAttachment(slist));
}
    
void        SqlQuery::attach(int n, StringSet& sset)
{
    setAttach(n, new StringSetAttachment(sset));
}

void        SqlQuery::attach(int n, Vector<QString>&svec)
{
    setAttach(n, new StringVectorAttachment(svec));
}

bool    SqlQuery::batch(bool fNoisy)
{
    bool    f   = QSqlQuery::execBatch();
    if(!f && fNoisy){
        dbWarning << "SQL Execute Error: " << lastError()
            << "\n(" << executedQuery() << ")";
    }
    return f;
}


void    SqlQuery::bind(int n, const QByteArray& val)
{
    bindValue(n, val);
}

void    SqlQuery::bind(int n, const QString&s)
{
    bindValue(n, s);
}

void    SqlQuery::bind(int n, const QVariant& val)
{
    bindValue(n, val);
}

void    SqlQuery::bind(int n, const String&s)
{
    bindValue(n, s.qString());
}

void    SqlQuery::bind(int n, uint64_t val)
{
    bindValue(n, (quint64) val);
}

bool    SqlQuery::exec(bool fNoisy)
{
    bool    f   = QSqlQuery::exec();
    if(!f && fNoisy){
        dbWarning << "SQL Execute Error: " << lastError()
            << "\n(" << executedQuery() << ")";
    }
    return f;
}

uint64_t    SqlQuery::lastInsertIdU64() const
{
    return (uint64_t) lastInsertId().toULongLong();
}


bool        SqlQuery::prepare(const QString&q)
{
    bool    f   = QSqlQuery::prepare(q);
    if(!f){
        dbWarning << "SQL Syntax Error: " << lastError()
            << "\n{" << q << "}";
    }
    return f;
}


void        SqlQuery::process()
{
    while(next()){
        for(auto& i : m_attach){
            if(i.second)
                i.second -> value( value(i.first));
        }
    }
}

void        SqlQuery::setAttach(int n, Attachment*att)
{
    Attachment* old = m_attach.replace(n, att);
    if(old)
        delete old;
}

bool        SqlQuery::valueBool(int n) const
{
    auto b = to_boolean(value(n).toString());
    return b.good ? b.value : false;
}

QByteArray  SqlQuery::valueBytes(int n) const
{
    return value(n).toByteArray();
}

QString     SqlQuery::valueString(int n) const
{
    return value(n).toString();
}

uint64_t    SqlQuery::valueU64(int n) const
{
    return (uint64_t) value(n).toULongLong();
}

//  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SqlQuery::AutoFinish::AutoFinish(AutoFinish&& mv) : q(mv.q)
{
    mv.q    = nullptr;
}

SqlQuery::AutoFinish& SqlQuery::AutoFinish::operator=(AutoFinish&& mv)
{
    if(this != &mv){
        if(q){
            q->finish();
            q->m_attach.clear();
        }
        q       = mv.q;
        mv.q    = nullptr;
    }
    return *this;
}

SqlQuery::AutoFinish::~AutoFinish()
{
    if(q){
        q -> finish();
        q   = nullptr;
    }
}

SqlQuery::AutoFinish::AutoFinish(SqlQuery* _q) : q(_q) {}

//  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


bool            db_run_script(const String&data, QSqlDatabase db)
{
    Vector<String>  lines;
    for(const String& s : data.split('\n')){
        if(is_blank(s))
            continue;
        if(s.trimmed().starts_with("--"))
            continue;
        lines << s;
    }
    
    lines   = String(' ').join(lines).split(';');
    bool    success = true;
    for(String s : lines){
        s = s.trimmed();
        if(s.empty())
            continue;
        QSqlQuery q(db);
        if(!q.exec(s.qString())){
            dbError << "SQL failure: " << q.lastError().text() << "\n" << s;
            success = false;
        }
    }
    return success;
}

bool            db_run_script_file(const char*file, QSqlDatabase db)
{
    return db_run_script(String(file_load_all(file)), db);
}

bool            db_run_script_file(const QString&str, QSqlDatabase db)
{
    std::string     f   = str.toStdString();
    return db_run_script(String(file_load_all(f.c_str())), db);
}


bool            db_table_exists(const String&tn, QSqlDatabase db)
{
    QString q  = tn.qString();
    for(QString t : db.tables())
        if(is_similar(t, q))
            return true;
    return false;
}

