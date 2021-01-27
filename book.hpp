#ifndef Bookstore_book
#define Bookstore_book

#include <iostream>
#include <cstdio>
#include <cstring>
#include <iomanip>

using namespace std;

class book {
    friend ostream& operator << (ostream &out, const book &obj) ;
private:
    char ISBN[25], name[65], author[65], keyword[70][70] ;
    int keyword_cnt, quantity, log_cnt ;
    double price ;
    double finance_log[110] ;

public:
    book () {
        memset (ISBN, 0, sizeof ISBN) ;
        memset (name, 0, sizeof name) ;
        memset (author, 0, sizeof author) ;
        memset (keyword, 0, sizeof keyword) ;
        keyword_cnt = quantity = price = log_cnt = 0 ;
    }
    book (const char *_ISBN) {
        memset (ISBN, 0, sizeof ISBN) ;
        memset (name, 0, sizeof name) ;
        memset (author, 0, sizeof author) ;
        memset (keyword, 0, sizeof keyword) ;
        keyword_cnt = quantity = price = log_cnt = 0 ;
        strcpy (ISBN, _ISBN) ;
    }

    book (const book &_book) {
        strcpy (ISBN, _book.ISBN) ;
        strcpy (name, _book.name) ;
        strcpy (author, _book.author) ;
        for (int i = 0; i < _book.keyword_cnt; i ++)
            strcpy (keyword[i], _book.keyword[i]) ;
        keyword_cnt = _book.keyword_cnt; quantity = _book.quantity; price = _book.price; log_cnt = _book.log_cnt ;
        for (int i = 0; i < _book.log_cnt; i ++) finance_log[i] = _book.finance_log[i] ;
    }

    book& operator = (const book &_book) {
        if (this == &_book) return *this ;
        strcpy (ISBN, _book.ISBN) ;
        strcpy (name, _book.name) ;
        strcpy (author, _book.author) ;
        for (int i = 0; i < _book.keyword_cnt; i ++)
            strcpy (keyword[i], _book.keyword[i]) ;
        keyword_cnt = _book.keyword_cnt; quantity = _book.quantity; price = _book.price; log_cnt = _book.log_cnt ;
        for (int i = 0; i < _book.log_cnt; i ++) finance_log[i] = _book.finance_log[i] ;
        return *this ;
    }

    bool operator < (const book &_book) const {
        return strcmp (ISBN, _book.ISBN) < 0 ;
    }

    void getISBN (char *_ISBN) {
        strcpy (_ISBN, ISBN) ;
    }
    void getName (char *_name) {
        strcpy (_name, name) ;
    }
    void getAuthor (char *_author) {
        strcpy (_author, author) ;
    }
    int getKeywordCount () const {
        return keyword_cnt ;
    }
    void getKeyword (int id, char *_keyword) {
        strcpy (_keyword, keyword[id]) ;
    }

    bool empty () {
        return strlen (ISBN) == 0 ;
    }
    
    void modify_ISBN (const char *_ISBN) {
        strcpy (ISBN, _ISBN) ;
    }

    void modify_name (const char *_name) {
        strcpy (name, _name) ;
    }

    void modify_author (const char *_author) {
        strcpy (author, _author) ;
    }

    void clear_keyword () {
        for (int i = 0; i < keyword_cnt; i ++) memset (keyword[i], 0, sizeof keyword[i]) ;
        keyword_cnt = 0 ;
    }

    void add_keyword (const char *_keyword) {
        strcpy (keyword[keyword_cnt ++], _keyword) ;
    }
    
    void modify_price (const double p) {
        price = p ;
    }

    void import (int q, double cost_price) {
        quantity += q ;
        finance_log[log_cnt ++] = -cost_price ;
    }

    void print_finance (double p) {
        if (p > 0) printf("+ ") ;
        else printf("- ") ;
        printf("%.2f ", p) ;
    }

    void show_finance (int cnt) {
        if (cnt == -1) {
            for (int i = 0; i < log_cnt; i ++) print_finance (finance_log[i]) ;
        } else {
            for (int i = max (0, log_cnt - cnt); i < log_cnt; i ++)
                print_finance (finance_log[i]) ;
        }
        printf("\n") ;
    }

    void buy (int q) {
        if (q > quantity) throw "no enough books" ;
        quantity -= q ;
        finance_log[log_cnt ++] = price * q ;
    }
} ;

ostream& operator << (ostream& out, const book &obj) {
    out << obj.ISBN << "\t" << obj.name << "\t" << obj.author << "\t" ;
    for (int i = 0; i < obj.keyword_cnt - 1; i ++) out << obj.keyword[i] << "|" ;
    out << obj.keyword[obj.keyword_cnt - 1] << "\t" ;
    out << fixed << setprecision (2) << obj.price << "\t" << obj.quantity ;
    return out ;
}

#endif