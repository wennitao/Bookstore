#ifndef Bookstore_book
#define Bookstore_book

#include <iostream>
#include <cstdio>
#include <cstring>

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

    void getISBN (char *_ISBN) {
        strcpy (_ISBN, ISBN) ;
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

    void modify_keyword (const char *keywords) {
        cout << keywords << endl ;
        keyword_cnt = 0 ;
        char tmp[70] ;
        int cur = 0 ;
        while (1) {
            int cur_word = 0 ;
            for (; keywords[cur] && keywords[cur] != '|'; cur ++, cur_word ++)
                tmp[cur_word] = keywords[cur] ;
            tmp[cur_word ++] = '\0' ;
            strcpy (keyword[keyword_cnt ++], tmp) ;
            if (!keywords[cur ++]) break ;
        }
    }
    
    void modify_price (const double p) {
        price = p ;
    }

} ;

ostream& operator << (ostream& out, const book &obj) {
    out << "ISBN:" << obj.ISBN << " name:" << obj.name << " author:" << obj.author << " keywords:" ;
    for (int i = 0; i < obj.keyword_cnt; i ++) out << obj.keyword[i] << " " ;
    out << "price:" << obj.price << " quantity:" << obj.quantity ;
    return out ;
}

#endif