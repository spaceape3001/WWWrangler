////////////////////////////////////////////////////////////////////////////////
//
//  YOUR QUILL
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <string_view>
#include <util/Map.hpp>
#include <util/NoCopy.hpp>
#include <util/NoMove.hpp>
#include <sys/inotify.h>

class DirWatcher : NoCopy, NoMove {
public:
    DirWatcher();
    ~DirWatcher();
    
    int         fd() const { return m_fd; }
    
    // returns FALSE if fatal error occurs
    bool        process();
    
    int         watch(const std::string&, uint32_t mask);
    int         descriptor(const std::string&);
    
    
protected:

    //  called when an event happened
    virtual void    dispatch(const std::string& watchedFile, const inotify_event& event, const std::string_view& name) {}
    void            diag_print(const std::string& watchedFile, const std::string_view& name);
    
private:

    bool operator<=>(const DirWatcher&) = delete;

    struct Watch {
        std::string     path;
        uint32_t        mask    = 0;
    };

    int                     m_fd;
    Map<int,Watch>          m_id2wd;
    Map<std::string,int>    m_name2id;
};
