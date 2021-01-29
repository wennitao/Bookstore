#ifndef Bookstore_user
#define Bookstore_user

#include <cstdio>
#include <iostream>
#include <cstring>

#include "B+Tree.hpp"

using namespace std;

class user {
    friend ostream& operator << (ostream &out, const user &_user) ;
private:
    char name[35], user_id[35], passwd[35] ;
    int privilege ;

public:
    user () {
        memset (name, 0, sizeof name) ;
        memset (user_id, 0, sizeof user_id) ;
        memset (passwd, 0, sizeof passwd) ;
        privilege = -1 ;
    }

    user (const char *_name, const char *_user_id, const char *_passwd, const int p) {
        strcpy (name, _name) ;
        strcpy (user_id, _user_id) ;
        strcpy (passwd, _passwd) ;
        privilege = p ;
    }

    user (const user &_user) {
        strcpy (name, _user.name) ;
        strcpy (user_id, _user.user_id) ;
        strcpy (passwd, _user.passwd) ;
        privilege = _user.privilege ;
    }

    user& operator = (const user &_user) {
        if (this == &_user) return *this ;
        strcpy (name, _user.name) ;
        strcpy (user_id, _user.user_id) ;
        strcpy (passwd, _user.passwd) ;
        privilege = _user.privilege ;
        return *this ;
    }

    bool operator == (const user &_user) {
        return strcmp (user_id, _user.user_id) == 0 ;
    }

    bool operator < (const user &_user) const {
        return strcmp (user_id, _user.user_id) < 0 ;
    }

    bool operator <= (const user &_user) const {
        return strcmp (user_id, _user.user_id) <= 0 ;
    }

    void login (const char *pass, int p) {
        if (p > privilege) return ;
        if (strcmp (pass, passwd) != 0) throw "Wrong password" ;
    }
    int getPrivilege () const {
        return privilege ;
    }
    void getUserid (char *_user_id) {
        strcpy (_user_id, user_id) ;
    }

    bool is_root () const {
        return strcmp (user_id, "root") == 0 ;
    }

    void updatePassword (const char *old_passwd, const char *new_passwd, bool is_root) {
        if (is_root) strcpy (passwd, new_passwd) ;
        else {
            if (strcmp (old_passwd, passwd) != 0) throw "Wrong password" ;
            strcpy (passwd, new_passwd) ;
        }
    }
} ;

ostream& operator << (ostream &out, const user &_user) {
    out << "name:" << _user.name << " user_id:" << _user.user_id << " passwd:" << _user.passwd << " privilege:" << _user.privilege ;
    return out ;
}

#endif