#include <iostream>
#include <cstdio>
#include <set>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "data.hpp"
#include "user.hpp"
#include "book.hpp"
#include "B+Tree.hpp"

using namespace std;

BPlusTree users ("users_B+Tree.dat") ;
BPlusTree books ("books_B+Tree.dat") ;
BPlusTree names ("book_name.dat") ;
BPlusTree authors ("book_author.dat") ;
BPlusTree keywords ("book_keyword.dat") ;

user root = user ("root", "root", "sjtu", 7) ;
user not_logged_in = user ("not_logged_in", "not_logged_in", "not_logged_in", 0) ;

stack<pair<user, int> > cur_status ;
pair<user, int> cur_st ;

fstream userio, bookio, financeio ;

//first-time launch init
void init () {
    //users init
    fstream in ("users.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("users.dat", ios::out | ios::binary) ;
        out.close() ;
        userio.open ("users.dat", ios::in | ios::out | ios::binary) ;
        userio.seekp (0, ios::end) ;
        char tmp_userid[35] ;
        not_logged_in.getUserid (tmp_userid) ;
        users.insert (data (tmp_userid, userio.tellp())) ;
        userio.write (reinterpret_cast<char *>(&not_logged_in), sizeof (not_logged_in)) ;
        root.getUserid (tmp_userid) ;
        users.insert (data (tmp_userid, userio.tellp())) ;
        userio.write (reinterpret_cast<char *>(&root), sizeof (root)) ;
    }
    in.close() ;
    if (!userio.is_open()) userio.open ("users.dat", ios::in | ios::out | ios::binary) ;
    cur_st = make_pair (not_logged_in, -1) ;
    cur_status.push (cur_st) ;

    //books init
    in.open ("books.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("books.dat", ios::out | ios::binary) ;
        out.close() ;
    }
    in.close() ;
    bookio.open ("books.dat", ios::in | ios::out | ios::binary) ;

    //finance init
    in.open ("finance.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("finance.dat", ios::out | ios::binary) ;
        int cnt = 0 ;
        out.write (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
        out.close() ;
    }
    in.close() ;
    financeio.open ("finance.dat", ios::in | ios::out | ios::binary) ;
}

//user commands
const char user_path[50] = "users.dat" ;

user user_read (int pos) {
    userio.seekg (pos, ios::beg) ;
    user cur ;
    userio.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    return cur ;
}
int user_write (user &cur) {
    userio.seekp (0, ios::end) ;
    int pos = userio.tellp() ;
    userio.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    return pos ;
}
void user_write (int pos, user &cur) {
    userio.seekp (pos, ios::beg) ;
    userio.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
}

void find (const char *user_id, vector<int> &pos) {
    users.find (data (user_id, 0), pos) ;
    if (pos.empty()) throw "user not found" ;
}

void login (const char *user_id, const char *passwd) {
    vector<int> pos ;
    users.find (data (user_id, 0), pos) ;
    if (pos.empty()) throw "user not found" ;
    user targ_user = user_read (pos[0]) ;
    targ_user.login (passwd, cur_st.first.getPrivilege()) ;
    cur_st = make_pair (targ_user, -1) ;
    cur_status.push (cur_st) ;
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
    if (cur_st.first.getPrivilege () < 7) throw "no enought privilege" ;
    if (strcmp (user_id, "root") == 0) throw "can not delete yourself" ;
    vector<int> pos ;
    users.find (data (user_id, 0), pos) ;
    if (pos.empty()) throw "user not found" ;
    users.erase (data (user_id, pos[0])) ;
}

void updatePasswd (const char *user_id, const char *old_passwd, const char *new_passwd) {
    if (cur_st.first.getPrivilege () < 1) throw "no enough privilege" ;
    vector<int> pos ;
    users.find (data (user_id, 0), pos) ;
    if (pos.empty()) throw "not found" ;
    user targ_user = user_read (pos[0]) ;

    bool is_root = cur_st.first.is_root () ;
    targ_user.updatePassword (old_passwd, new_passwd, is_root) ;
    user_write (pos[0], targ_user) ;
}

//book commands
const char book_path[50] = "books.dat" ;

book book_read (int pos) {
    if (pos == -1) return book () ;
    bookio.seekg (pos, ios::beg) ;;
    book cur ;
    bookio.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    return cur ;
}
int book_write (book &cur) {
    bookio.seekp (0, ios::end) ;
    int pos = bookio.tellp() ;
    bookio.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
    return pos ;
}
void book_write (int pos, book &cur) {
    bookio.seekp (pos, ios::beg) ;
    bookio.write (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
}

int read_integer (const char *num) {
    int x = 0, len = strlen (num) ;
    for (int i = 0; i < len; i ++) 
        if (num[i] < '0' || num[i] > '9') throw "not a number" ;
    for (int i = 0; i < len; i ++) x = x * 10 + num[i] - '0' ;
    return x ;
}
double read_double (const char *num) {
    double p = 0, pdot = 0, now = 0.1; int i = 0, len = strlen (num);
    for (int i = 0; i < len; i ++) 
        if (num[i] != '.' && (num[i] < '0' || num[i] > '9')) throw "not a number" ;
    for (; i < len && num[i] != '.' && num[i]; i ++) p = p * 10 + num[i] - '0' ;
    for (i ++; i < len && num[i]; i ++) pdot += now * (num[i] - '0'), now *= 0.1 ;
    return p + pdot ;
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
        cur_st.second = write_pos ;
    } else {
        cur_st.second = pos[0] ;
    }
    cur_status.top() = cur_st ;
}

void modify (const char *op_str) {
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;
    book cur_book = book_read (cur_st.second) ;
    if (cur_book.empty()) throw "book not selected" ;

    int len = strlen (op_str) ;
    char op[110] = {0}; int i = 1, op_len = 0 ;
    for (; i < len && op_str[i] != '='; i ++) op[op_len ++] = op_str[i] ;
    if (i == len || i == len - 1) throw "wrong format" ;

    string content ;
    for (i ++; i < len; i ++) content += op_str[i] ;

    if (strcmp (op, "ISBN") == 0) {
        char ISBN[30] = {0} ;
        strcpy (ISBN, content.c_str()) ;
        vector<int> pos ;
        books.find (data (ISBN, 0), pos) ;
        if (!pos.empty()) throw "ISBN already exists" ;
        
        cur_book.getISBN (ISBN) ;
        books.erase (data (ISBN, cur_st.second)) ;
        strcpy (ISBN, content.c_str()) ;
        cur_book.modify_ISBN (ISBN) ;
        books.insert (data (ISBN, cur_st.second)) ;

    } else if (strcmp (op, "name") == 0) {
        char name[70] = {0} ;
        cur_book.getName (name) ;
        if (strlen (name)) {
            names.erase (data (name, cur_st.second)) ;
        }
        strcpy (name, content.substr (1, content.length() - 2).c_str()) ;
        cur_book.modify_name (name) ;
        names.insert (data (name, cur_st.second)) ;

    } else if (strcmp (op, "author") == 0) {
        char author[70] = {0} ;
        cur_book.getAuthor (author) ;
        if (strlen (author)) {
            authors.erase (data (author, cur_st.second)) ;
        }
        strcpy (author, content.substr (1, content.length() - 2).c_str()) ;
        cur_book.modify_author (author) ;
        authors.insert (data (author, cur_st.second)) ;

    } else if (strcmp (op, "keyword") == 0) {
        int cur = 0, cnt = 0 ;
        char s[70][70] = {0}, keyword[70] = {0}, tmp[70] = {0} ;
        strcpy (keyword, content.substr (1, content.length() - 2).c_str()) ;
        while (1) {
            int cur_word = 0 ;
            for (; keyword[cur] && keyword[cur] != '|'; cur ++, cur_word ++)
                tmp[cur_word] = keyword[cur] ;
            tmp[cur_word ++] = '\0' ;
            strcpy (s[cnt ++], tmp) ;
            if (!keyword[cur ++]) break ;
        }
        for (int i = 0; i < cnt; i ++)
            for (int j = i + 1; j < cnt; j ++)
                if (strcmp (s[i], s[j]) == 0) throw "repeated keywords" ;

        int pre_cnt = cur_book.getKeywordCount() ;
        for (int i = 0; i < pre_cnt; i ++) {
            cur_book.getKeyword (i, tmp) ;
            keywords.erase (data (tmp, cur_st.second)) ;
        }
        cur_book.clear_keyword() ;
        
        for (int i = 0; i < cnt; i ++) {
            cur_book.add_keyword (s[i]) ;
            keywords.insert (data (s[i], cur_st.second)) ;
        }
    } else if (strcmp (op, "price") == 0) {
        double price = read_double (content.c_str()) ;
        cur_book.modify_price (price) ;
    } else {
        throw "wrong format" ;
    }

    book_write (cur_st.second, cur_book) ;
}

void add_finance_log (double p) ;
void import_book (int quantity, double cost_price) {
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;
    book cur_book = book_read (cur_st.second) ;
    if (cur_book.empty()) throw "book not selected" ;
    cur_book.import (quantity) ;
    book_write (cur_st.second, cur_book) ;
    
    add_finance_log (-cost_price) ;
}

void show (const char *op_str) {
    if (cur_st.first.getPrivilege() < 1) throw "no enough privilege" ;
    vector<int> pos ;
    vector<book> all_books ;
    if (strlen (op_str)) {
        int len = strlen (op_str) ;
        char op[110] = {0}; int i = 1, op_len = 0 ;
        for (; i < len && op_str[i] != '='; i ++) op[op_len ++] = op_str[i] ;
        if (i == len || i == len - 1) throw "wrong format" ;

        string content ;
        for (i ++; i < len; i ++) content += op_str[i] ;

        if (strcmp (op, "ISBN") == 0) {
            char ISBN[70] = {0} ;
            strcpy (ISBN, content.c_str()) ;
            books.find (data (ISBN, 0), pos) ;
        } else if (strcmp (op, "name") == 0) {
            char name[70] = {0} ;
            strcpy (name, content.substr (1, content.length() - 2).c_str()) ;
            names.find (data (name, 0), pos) ;
        } else if (strcmp (op, "author") == 0) {
            char author[70] = {0} ;
            strcpy (author, content.substr (1, content.length() - 2).c_str()) ;
            authors.find (data (author, 0), pos) ;
        } else if (strcmp (op, "keyword") == 0) {
            char keyword[70] = {0} ;
            strcpy (keyword, content.substr (1, content.length() - 2).c_str()) ;
            keywords.find (data (keyword, 0), pos) ;
        } else {
            throw "wrong format" ;
        }
        for (int i = 0; i < pos.size(); i ++)
            all_books.push_back (book_read (pos[i])) ;
    } else { //show all
        book cur ;
        bookio.seekg (0, ios::beg) ;
        while (bookio.peek() != EOF && bookio.read (reinterpret_cast<char *>(&cur), sizeof (cur))) {
            all_books.push_back (cur) ;
        }
    }
    sort (all_books.begin(), all_books.end()) ;
    for (int i = 0; i < all_books.size(); i ++) cout << all_books[i] << endl ;
    if (all_books.empty()) cout << endl ;
}


//finance commands
const char finance_path[50] = "finance.dat" ;

void print_finance_log (int cnt) {
    financeio.seekg (0, ios::beg) ;
    int log_cnt ;
    financeio.read (reinterpret_cast<char *>(&log_cnt), sizeof log_cnt) ;
    if (cnt < 1e8 && cnt > log_cnt) throw "no enough logs" ;
    double all_pos = 0, all_neg = 0 ;
    for (int i = max (0, log_cnt - cnt); i < log_cnt; i ++) {
        double p ;
        financeio.seekg (sizeof (log_cnt) + i * sizeof (p)) ;
        financeio.read (reinterpret_cast<char *>(&p), sizeof p) ;
        if (p > 0) all_pos += p ;
        else all_neg += fabs (p) ;
    }
    printf("+ %.2f - %.2f\n", all_pos, all_neg) ;
}

void add_finance_log (double p) {
    financeio.seekp (0, ios::end) ;
    financeio.write (reinterpret_cast<char *>(&p), sizeof p) ;
    financeio.seekg (0, ios::beg) ;
    int cnt ;
    financeio.read (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
    cnt ++ ;
    financeio.seekp (0, ios::beg) ;
    financeio.write (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
}

void show_finance (int cnt) {
    if (cur_st.first.getPrivilege() < 7) throw "no enough privilege" ;
    print_finance_log (cnt) ;
}

void buy (const char *ISBN, int quantity) {
    vector<int> pos ;
    books.find (data (ISBN, 0), pos) ;
    if (pos.empty()) throw "book not found" ;
    book cur = book_read (pos[0]) ;
    cur.buy (quantity) ;
    book_write (pos[0], cur) ;

    add_finance_log (cur.getPrice() * quantity) ;
}

void close_files() {
    userio.close(); bookio.close(); financeio.close() ;
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
        try {
            if (strcmp (op, "su") == 0) {
                if (cnt > 3 || cnt < 2) throw "wrong command" ;
                if (cnt == 3) login (tmp[1], tmp[2]) ; //user_id, passwd
                else login (tmp[1], "") ;
            } else if (strcmp (op, "logout") == 0) {
                if (cnt != 1) throw "wrong command" ;
                logout () ;
            } else if (strcmp (op, "useradd") == 0) {
                if (cnt != 5) throw "wrong command" ;
                int p = 0 ;
                for (int i = 0; tmp[3][i]; i ++) p = p * 10 + tmp[3][i] - '0' ;
                useradd (tmp[1], tmp[2], p, tmp[4]) ;
            } else if (strcmp (op, "register") == 0) {
                if (cnt != 4) throw "wrong command" ;
                registerUser (tmp[1], tmp[2], tmp[3]) ;
            } else if (strcmp (op, "delete") == 0) {
                if (cnt != 2) throw "wrong command" ;
                deleteUser (tmp[1]) ;
            } else if (strcmp (op, "passwd") == 0) {
                if (cnt < 3 || cnt > 4) throw "wrong command" ;
                if (cnt == 3) updatePasswd (tmp[1], "", tmp[2]) ;
                else updatePasswd (tmp[1], tmp[2], tmp[3]) ;
            } else if (strcmp (op, "select") == 0) {
                if (cnt != 2) throw "wrong command" ;
                select (tmp[1]) ;
            } else if (strcmp (op, "modify") == 0) {
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
            } else if (strcmp (op, "import") == 0) {
                if (cnt != 3) throw "wrong command" ;
                int quantity = read_integer (tmp[1]); double cost_price = read_double (tmp[2]) ;
                import_book (quantity, cost_price) ;
            } else if (strcmp (op, "show") == 0) {
                if (cnt == 1) show ("") ;
                else if (cnt == 2) {
                    if (tmp[1][0] == 'f') show_finance (1e9) ;
                    else show (tmp[1]) ;
                }
                else {
                    if (cnt == 3) {
                        if (tmp[1][0] != 'f') throw "wrong command" ;
                        else show_finance (read_integer (tmp[2])) ;
                    } else {
                        throw "wrong command" ;
                    }
                }
            } else if (strcmp (op, "buy") == 0) {
                if (cnt != 3) throw "wrong command" ;
                buy (tmp[1], read_integer (tmp[2])) ;
            } else if (strcmp (op, "exit") == 0) {
                if (cnt != 1) throw "wrong command" ;
                close_files() ;
                break ;
            } else if (strcmp (op, "quit") == 0) {
                if (cnt != 1) throw "wrong command" ;
                close_files() ;
                break ;
            } else {
                throw "wrong command" ;
            }
        } catch (...) {
            printf("Invalid\n") ;
        }
    }
}

int main() {
    init () ;
    runCommands () ;
    return 0 ;
}