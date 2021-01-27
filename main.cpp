#include <iostream>
#include <cstdio>
#include <set>
#include <vector>
#include <stack>
#include <sstream>

#include "data.hpp"
#include "user.hpp"
#include "book.hpp"
#include "B+Tree.hpp"

using namespace std;

BPlusTree users ("users_B+Tree.dat") ;
BPlusTree books ("books_B+Tree.dat") ;

user root = user ("root", "root", "sjtu", 7) ;
user not_logged_in = user ("not_logged_in", "not_logged_in", "not_logged_in", 0) ;

stack<pair<user, book> > cur_status ;
pair<user, book> cur_st ;

//first-time launch init
void init () {
    fstream in ;
    in.open ("users.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ;
        out.open ("users.dat", ios::out | ios::binary) ;
        out.close () ;
        out.open ("users.dat", ios::in | ios::out | ios::binary) ;
        out.seekp (0, ios::end) ;
        char tmp_userid[35] ;
        not_logged_in.getUserid (tmp_userid) ;
        users.insert (data (tmp_userid, out.tellp())) ;
        out.write (reinterpret_cast<char *>(&not_logged_in), sizeof (not_logged_in)) ;
        root.getUserid (tmp_userid) ;
        users.insert (data (tmp_userid, out.tellp())) ;
        out.write (reinterpret_cast<char *>(&root), sizeof (root)) ;
    }
    cur_status.push (make_pair (not_logged_in, book())) ;
    cur_st = make_pair (not_logged_in, book()) ;
    in.close() ;
    in.open ("books.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("books.dat", ios::out | ios::binary) ;
        out.close() ;
    }
}

//user commands
const char user_path[50] = "users.dat" ;

user user_read (int pos) {
    fstream in (user_path, ios::in | ios::binary) ;
    in.seekg (pos, ios::beg) ;
    user cur ;
    in.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    in.close() ;
    return cur ;
}
int user_write (user &cur) {
    fstream out (user_path, ios::in | ios::out | ios::binary) ;
    out.seekp (0, ios::end) ;
    int pos = out.tellp() ;
    out.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    out.close() ;
    return pos ;
}
void user_write (int pos, user &cur) {
    fstream out (user_path, ios::in | ios::out | ios::binary) ;
    out.seekp (pos, ios::beg) ;
    out.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    out.close() ;
}

void find (const char *user_id, vector<int> &pos) {
    users.find (data (user_id, 0), pos) ;
    if (pos.empty() == 0) throw "user not found" ;
}

void login (const char *user_id, const char *passwd) {
    try {
        vector<int> pos ;
        users.find (data (user_id, 0), pos) ;
        if (pos.empty()) throw "not found" ;
        user targ_user = user_read (pos[0]) ;
        targ_user.login (passwd) ;
        cur_status.push (make_pair (targ_user, book())) ;
        cur_st = make_pair (targ_user, book()) ;
    } catch (...) {
        printf("Invalid\n") ;
    }
}

void logout () {
    if (cur_st.first.getPrivilege () < 1) throw "Can not logout" ;
    cur_status.pop(); cur_st = cur_status.top() ;
}

void addUser (const char *user_id, const char *passwd, const int privilege, const char *name) {
    user new_user = user (name, user_id, passwd, privilege) ;
    int pos = user_write (new_user) ;
    users.insert (data (user_id, pos)) ;
}

void useradd (const char *user_id, const char *passwd, const int privilege, const char *name) {
    if (privilege != 7 && privilege != 3 && privilege != 1) throw "privilege invalid" ;
    if (cur_st.first.getPrivilege() <= privilege) throw "no enough privilege" ;
    vector<int> pos ;
    users.find (data (user_id, 0), pos) ;
    if (!pos.empty()) throw "user already exists" ;
    addUser (user_id, passwd, privilege, name) ;
}

void registerUser (const char *user_id, const char *passwd, const char *name) {
    addUser (user_id, passwd, 1, name) ;
}

void deleteUser (const char *user_id) {
    vector<int> pos ;
    users.find (data (user_id, 0), pos) ;
    if (pos.empty()) throw "user not found" ;
    try {
        users.erase (data (user_id, pos[0])) ;
    } catch (...) {
        printf("Invalid\n") ;
    }
}

void updatePasswd (const char *user_id, const char *old_passwd, const char *new_passwd) {
    if (cur_st.first.getPrivilege () < 1) throw "no enough privilege" ;
    try {
        vector<int> pos ;
        users.find (data (user_id, 0), pos) ;
        if (pos.empty()) throw "not found" ;
        user targ_user = user_read (pos[0]) ;
        try {
            bool is_root = cur_st.first.is_root () ;
            targ_user.updatePassword (old_passwd, new_passwd, is_root) ;
            user_write (pos[0], targ_user) ;
        } catch (...) {
            printf("Invalid\n") ;
        }
    } catch (...) {
        printf("Invalid\n") ;
    }
}

//book commands
const char book_path[50] = "books.dat" ;
int cur_book_pos ;

book book_read (int pos) {
    fstream in (book_path, ios::in | ios::binary) ;
    in.seekg (pos, ios::beg) ;
    book cur ;
    in.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    in.close() ;
    return cur ;
}
int book_write (book &cur) {
    fstream out (book_path, ios::in | ios::out | ios::binary) ;
    out.seekp (0, ios::end) ;
    int pos = out.tellp() ;
    out.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    out.close() ;
    return pos ;
}
void book_write (int pos, book &cur) {
    fstream out (book_path, ios::in | ios::out | ios::binary) ;
    out.seekp (pos, ios::beg) ;
    out.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    out.close() ;
}

void select (const char *ISBN) {
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;

    vector<int> pos ;
    books.find (data (ISBN, 0), pos) ;
    book cur ;
    if (pos.empty()) {
        cur = book (ISBN) ;
        int write_pos = book_write (cur) ;
        books.insert (data (ISBN, write_pos)) ;
        cur_book_pos = write_pos ;
    } else {
        cur = book_read (pos[0]) ;
        cur_book_pos = pos[0] ;
    }
    cur_st.second = cur; cur_status.top() = cur_st ;

    cout << cur << endl ;
}

void modify (const char *op_str) {
    cout << op_str << endl ;
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;
    book &cur_book = cur_st.second ;
    if (cur_book.empty()) throw "book not selected" ;

    int len = strlen (op_str) ;
    char op[110] = {0}; int i = 1, op_len = 0 ;
    for (; i < len && op_str[i] != '='; i ++) op[op_len ++] = op_str[i] ;
    if (i == len || i == len - 1) throw "wrong format" ;

    string content ;
    for (i ++; i < len; i ++) content += op_str[i] ;

    if (strcmp (op, "ISBN") == 0) {
        char tmp[30] ;
        cur_book.getISBN (tmp) ;
        books.erase (data (tmp, cur_book_pos)) ;
        cur_book.modify_ISBN (content.c_str()) ;
        books.insert (data (content.c_str(), cur_book_pos)) ;
    } else if (strcmp (op, "name") == 0) {
        cur_book.modify_name (content.substr (1, content.length() - 2).c_str()) ;
    } else if (strcmp (op, "author") == 0) {
        cur_book.modify_author (content.substr (1, content.length() - 2).c_str()) ;
    } else if (strcmp (op, "keyword") == 0) {
        cur_book.modify_keyword (content.substr (1, content.length() - 2).c_str()) ;
    } else if (strcmp (op, "price") == 0) {
        double p = 0, pdot = 0, now = 0.1; int i = 0 ;
        for (; i < content.length() && content[i] != '.' && content[i]; i ++) p = p * 10 + content[i] - '0' ;
        for (i ++; i < content.length() && content[i]; i ++) pdot += now * (content[i] - '0'), now *= 0.1 ;
        cur_book.modify_price (p + pdot) ;
    } else {
        throw "wrong format" ;
    }

    cout << cur_book << endl ;
    book_write (cur_book_pos, cur_book) ;
}

void runCommands () {
    string op_string, tmp_op_string ;
    while (getline (cin, op_string)) {
        tmp_op_string = op_string ;
        stringstream ss (op_string) ;
        char tmp[110][110]; int cnt = 0 ;
        while (ss >> tmp[cnt]) cnt ++ ;
        char op[20] ;
        strcpy (op, tmp[0]) ;
        if (strcmp (op, "su") == 0) {
            if (cnt != 3) {
                printf("Invalid\n"); continue ;
            }
            try {
                login (tmp[1], tmp[2]) ; //user_id, passwd
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "logout") == 0) {
            if (cnt != 1) {
                printf("Invalid\n"); continue ;
            }
            try {
                logout () ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "useradd") == 0) {
            if (cnt != 5) {
                printf("Invalid\n"); continue ;
            }
            int p = 0 ;
            for (int i = 0; tmp[3][i]; i ++) p = p * 10 + tmp[3][i] - '0' ;
            try {
                useradd (tmp[1], tmp[2], p, tmp[4]) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "register") == 0) {
            if (cnt != 4) {
                printf("Invalid\n"); continue ;
            }
            try {
                registerUser (tmp[1], tmp[2], tmp[3]) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "delete") == 0) {
            if (cnt != 2) {
                printf("Invalid\n"); continue ;
            }
            try {
                deleteUser (tmp[1]) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "passwd") == 0) {
            if (cnt < 3 || cnt > 4) {
                printf("Invalid\n"); continue ;
            }
            try {
                if (cnt == 3) updatePasswd (tmp[1], "", tmp[2]) ;
                else updatePasswd (tmp[1], tmp[2], tmp[3]) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "select") == 0) {
            if (cnt != 2) {
                printf("Invalid\n"); continue ;
            }
            try {
                select (tmp[1]) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "modify") == 0) {
            try {
                string ops[110] ;
                int pos = 0, opcnt = 0 ;
                for (; pos < tmp_op_string.length() && tmp_op_string[pos] != '-'; pos ++) ;
                while (pos < tmp_op_string.length()) {
                    int nxt = pos + 1; bool in = 0 ;
                    while (nxt < tmp_op_string.length()) {
                        if (tmp_op_string[nxt] == '-' && !in) break ;
                        if (tmp_op_string[nxt] == '"') in = !in ;
                        nxt ++ ;
                    }
                    if (nxt != tmp_op_string.length()) ops[opcnt ++] = tmp_op_string.substr (pos, nxt - pos - 1) ;
                    else ops[opcnt ++] = tmp_op_string.substr (pos, nxt - pos) ;
                    pos = nxt ;
                }
                for (int i = 0; i < opcnt; i ++) modify (ops[i].c_str()) ;
            } catch (...) {
                printf("Invalid\n") ;
            }
        } else if (strcmp (op, "import") == 0) {

        } else if (strcmp (op, "show") == 0) {

        } else if (strcmp (op, "buy") == 0) {

        } else if (strcmp (op, "exit") == 0) {
            if (cnt != 1) {
                printf("Invalid\n"); continue ;
            }
            break ;
        } else {
            printf("Invalid\n") ;
        }

        //users.print() ;
    }
}

int main() {
    init () ;
    //users.print() ;
    runCommands () ;
    return 0 ;
}