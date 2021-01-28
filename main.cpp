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
BPlusTree names ("book_name.dat") ;
BPlusTree authors ("book_author.dat") ;
BPlusTree keywords ("book_keyword.dat") ;

user root = user ("root", "root", "sjtu", 7) ;
user not_logged_in = user ("not_logged_in", "not_logged_in", "not_logged_in", 0) ;

stack<pair<user, book> > cur_status ;
pair<user, book> cur_st ;
int cur_book_pos ;

//first-time launch init
void init () {
    //users init
    fstream in ;
    in.open ("users.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        //printf("start init\n") ;
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

    //books init
    in.open ("books.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("books.dat", ios::out | ios::binary) ;
        out.close() ;
    }

    //finance init
    in.open ("finance.dat", ios::in | ios::binary) ;
    if (!in.is_open()) {
        fstream out ("finance.dat", ios::out | ios::binary) ;
        int cnt = 0 ;
        out.write (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
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
    vector<int> pos ;
    char ISBN[30] ;
    cur_st.second.getISBN (ISBN) ;
    books.find (data (ISBN, 0), pos) ;
    if (pos.empty()) cur_book_pos = -1 ;
    else cur_book_pos = pos[0] ;
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

int read_integer (const char *num) {
    int x = 0, len = strlen (num) ;
    for (int i = 0; i < len; i ++) x = x * 10 + num[i] - '0' ;
    return x ;
}

double read_double (const char *num) {
    double p = 0, pdot = 0, now = 0.1; int i = 0, len = strlen (num);
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
        cur_book_pos = write_pos ;
    } else {
        cur = book_read (pos[0]) ;
        cur_book_pos = pos[0] ;
    }
    cur_st.second = cur; cur_status.top() = cur_st ;

    //cout << cur << endl ;
    //books.print() ;
}

void modify (const char *op_str) {
    //cout << op_str << endl ;
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;
    book &cur_book = cur_st.second ;
    if (cur_book.empty()) throw "book not selected" ;

    int len = strlen (op_str) ;
    char op[110] = {0}; int i = 1, op_len = 0 ;
    for (; i < len && op_str[i] != '='; i ++) op[op_len ++] = op_str[i] ;
    if (i == len || i == len - 1) throw "wrong format" ;

    string content ;
    for (i ++; i < len; i ++) content += op_str[i] ;

    //cout << op << " " << content << endl ;

    if (strcmp (op, "ISBN") == 0) {
        char ISBN[30] ;
        strcpy (ISBN, content.c_str()) ;
        vector<int> pos ;
        books.find (data (ISBN, 0), pos) ;
        if (!pos.empty()) throw "ISBN already exists" ;

        cur_book.getISBN (ISBN) ;
        books.erase (data (ISBN, cur_book_pos)) ;
        strcpy (ISBN, content.c_str()) ;
        cur_book.modify_ISBN (ISBN) ;
        books.insert (data (ISBN, cur_book_pos)) ;

        //books.print() ;
    } else if (strcmp (op, "name") == 0) {
        //printf("modify name\n") ;
        char name[70] ;
        cur_book.getName (name) ;
        if (strlen (name)) {
            names.erase (data (name, cur_book_pos)) ;
        }
        strcpy (name, content.substr (1, content.length() - 2).c_str()) ;
        cur_book.modify_name (name) ;
        names.insert (data (name, cur_book_pos)) ;

        //names.print() ;
    } else if (strcmp (op, "author") == 0) {
        //printf("modify author\n") ;
        //cout << op << " " << content << endl ;
        char author[70] ;
        cur_book.getAuthor (author) ;
        if (strlen (author)) {
            authors.erase (data (author, cur_book_pos)) ;
        }
        strcpy (author, content.substr (1, content.length() - 2).c_str()) ;
        cur_book.modify_author (author) ;
        authors.insert (data (author, cur_book_pos)) ;

        //authors.print() ;
    } else if (strcmp (op, "keyword") == 0) {
        //printf("modify keyword\n") ;
        int cnt = cur_book.getKeywordCount() ;
        char keyword[70], tmp[70] ;
        for (int i = 0; i < cnt; i ++) {
            cur_book.getKeyword (i, tmp) ;
            //printf("delete %s %d\n", tmp, cur_book_pos) ;
            keywords.erase (data (tmp, cur_book_pos)) ;
            //keywords.print() ;
            //printf("--------------------------------\n") ;
        }
        cur_book.clear_keyword() ;

        strcpy (keyword, content.substr (1, content.length() - 2).c_str()) ;
        int cur = 0 ;
        while (1) {
            int cur_word = 0 ;
            for (; keyword[cur] && keyword[cur] != '|'; cur ++, cur_word ++)
                tmp[cur_word] = keyword[cur] ;
            tmp[cur_word ++] = '\0' ;
            cur_book.add_keyword (tmp) ;
            //printf("insert %s %d\n", tmp, cur_book_pos) ;
            keywords.insert (data (tmp, cur_book_pos)) ;
            //keywords.print() ;            
            //printf("--------------------------------\n") ;
            if (!keyword[cur ++]) break ;
        }
    } else if (strcmp (op, "price") == 0) {
        double price = read_double (content.c_str()) ;
        cur_book.modify_price (price) ;
    } else {
        throw "wrong format" ;
    }

    //cout << cur_book << endl ;
    book_write (cur_book_pos, cur_book) ;
}

void add_finance_log (double p) ;
void import_book (int quantity, double cost_price) {
    if (cur_st.first.getPrivilege() < 3) throw "no enough privilege" ;
    book &cur_book = cur_st.second ;
    if (cur_book.empty()) throw "book not selected" ;
    cur_book.import (quantity) ;
    book_write (cur_book_pos, cur_book) ;
    
    add_finance_log (-cost_price) ;
}

void show (const char *op_str) {
    if (cur_st.first.getPrivilege() < 1) throw "no enough privilege" ;
    vector<int> pos ;
    vector<book> all_books ;
    fstream in (book_path, ios::in | ios::binary) ;
    if (strlen (op_str)) {
        int len = strlen (op_str) ;
        char op[110] = {0}; int i = 1, op_len = 0 ;
        for (; i < len && op_str[i] != '='; i ++) op[op_len ++] = op_str[i] ;
        if (i == len || i == len - 1) throw "wrong format" ;

        string content ;
        for (i ++; i < len; i ++) content += op_str[i] ;

        if (strcmp (op, "ISBN") == 0) {
            char ISBN[70] ;
            strcpy (ISBN, content.c_str()) ;
            books.find (data (ISBN, 0), pos) ;
        } else if (strcmp (op, "name") == 0) {
            char name[70] ;
            strcpy (name, content.substr (1, content.length() - 2).c_str()) ;
            names.find (data (name, 0), pos) ;
        } else if (strcmp (op, "author") == 0) {
            char author[70] ;
            strcpy (author, content.substr (1, content.length() - 2).c_str()) ;
            authors.find (data (author, 0), pos) ;
        } else if (strcmp (op, "keyword") == 0) {
            char keyword[70] ;
            strcpy (keyword, content.substr (1, content.length() - 2).c_str()) ;
            keywords.find (data (keyword, 0), pos) ;
        } else {
            in.close() ;
            throw "wrong format" ;
        }
        for (int i = 0; i < pos.size(); i ++)
            all_books.push_back (book_read (pos[i])) ;
    } else { //show all
        book cur ;
        while (in.read (reinterpret_cast<char *>(&cur), sizeof (cur))) {
            all_books.push_back (cur) ;
        }
    }
    sort (all_books.begin(), all_books.end()) ;
    for (int i = 0; i < all_books.size(); i ++) cout << all_books[i] << endl ;
    if (all_books.empty()) cout << endl ;
    in.close() ;
}


//finance commands
const char finance_path[50] = "finance.dat" ;

void print_finance_log (int cnt) {
    fstream in (finance_path, ios::in | ios::binary) ;
    in.seekg (0, ios::beg) ;
    int log_cnt ;
    in.read (reinterpret_cast<char *>(&log_cnt), sizeof log_cnt) ;
    double all_pos = 0, all_neg = 0 ;
    for (int i = max (0, log_cnt - cnt); i < log_cnt; i ++) {
        double p ;
        in.seekg (sizeof (log_cnt) + i * sizeof (p)) ;
        in.read (reinterpret_cast<char *>(&p), sizeof p) ;
        if (p > 0) all_pos += p ;
        else all_neg += fabs (p) ;
    }
    printf("+ %.2f - %.2f\n", all_pos, all_neg) ;
}

void add_finance_log (double p) {
    fstream out (finance_path, ios::in | ios::out | ios::binary) ;
    out.seekp (0, ios::end) ;
    out.write (reinterpret_cast<char *>(&p), sizeof p) ;
    out.seekg (0, ios::beg) ;
    int cnt ;
    out.read (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
    cnt ++ ;
    out.seekp (0, ios::beg) ;
    out.write (reinterpret_cast<char *>(&cnt), sizeof cnt) ;
    out.close() ;
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
            if (cnt != 3) {
                printf("Invalid\n"); continue ;
            }
            try {
                int quantity = read_integer (tmp[1]); double cost_price = read_double (tmp[2]) ;
                import_book (quantity, cost_price) ;
            } catch (...) {
                printf("Invalid\n"); continue ;
            }
        } else if (strcmp (op, "show") == 0) {
            try {
                if (cnt == 1) show ("") ;
                else if (cnt == 2) {
                    if (tmp[1][0] == 'f') show_finance (2e9) ;
                    else show (tmp[1]) ;
                }
                else {
                    if (cnt == 3) {
                        if (tmp[1][0] != 'f') {
                            printf("Invalid\n"); continue ;
                        } else {
                            show_finance (read_integer (tmp[2])) ;
                        }
                    } else {
                        printf("Invalid\n"); continue ;
                    }
                }
            } catch (...) {
                printf("Invalid\n"); continue ;
            }
        } else if (strcmp (op, "buy") == 0) {
            if (cnt != 3) {
                printf("Invalid\n"); continue ;
            }
            try {
                buy (tmp[1], read_integer (tmp[2])) ;
            } catch (...) {
                printf("Invalid\n"); continue ;
            }
        } else if (strcmp (op, "exit") == 0) {
            if (cnt != 1) {
                printf("Invalid\n"); continue ;
            }
            break ;
        } else if (strcmp (op, "quit") == 0) {
            if (cnt != 1) {
                printf("Invalid\n"); continue ;
            }
            break ;
        } else {
            printf("Invalid\n") ;
        }

        //printf("users:\n"); users.print(); cout << endl ;
        //printf("books:\n"); books.print(); cout << endl ;
        //printf("names:\n"); names.print(); cout << endl ;
        //printf("authors:\n"); authors.print(); cout << endl ;
        //printf("keywords:\n"); keywords.print();
        //printf("------------------------\n") ;
    }
}

int main() {
    init () ;
    //users.print() ;

    runCommands () ;
    /*for (int t = 0; t < 1000; t ++) {
        printf("Test %d\n", t) ;
        system ("rm *.dat") ;
        BPlusTree test ("test.dat") ;
        int seed = time (0) ;
        //int seed = 1611835925 ;
        srand (seed) ;
        char s[100010][20] ;
        memset (s, 0, sizeof s) ;
        int n = 30 ;
        for (int i = 1; i <= n; i ++) {
            int len = 1 + rand() % 10 ;
            for (int j = 0; j < len; j ++)
                s[i][j] = 'a' + rand() % 26 ;
            //printf("insert (%s %d)\n", s[i], i) ;
            test.insert (data (s[i], i)) ;
            //test.print();
            //printf("----------------------------\n") ;
        }
        for (int i = 1; i <= n; i ++) {
            //printf("erase (%s %d)\n", s[i], i) ;
            test.erase (data  (s[i], i)) ;
            //test.print() ;
            //printf("----------------------------\n") ;
            for (int j = i + 1; j <= n; j ++) {
                data res = test.findKey (data (s[j], j)) ;
                if (j != res.pos) {
                    printf("seed:%d test:(%s %d) %d\n", seed, s[j], j, res.pos) ;
                    puts ("QAQ") ;
                    return 0 ;
                }
            }
        }
        for (int i = 1; i <= n; i ++) {
            data res = test.findKey (data (s[i], i)) ;
            if (i != res.pos) {
                //printf("del:%d cur:%d\n", j, i) ;
                printf("seed:%d test:(%s %d) %d\n", seed, s[i], i, res.pos) ;
                puts ("QAQ") ;
                return 0 ;
            }
        }
        
        //printf("\n") ;
    }*/
    //int seed = 1611818624 ;*/
    return 0 ;
}


//complex test 1.in line 142