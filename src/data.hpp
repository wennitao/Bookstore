#ifndef data_type
#define data_type

#include <cstring>

struct data {
    char str[110]; int pos ;
    data () {
        memset (str, 0, sizeof str) ;
        pos = -1 ;
    }
    data (const char *_str, int p) {
        strcpy (str, _str) ;
        pos = p ;
    }
    void print() const {
        printf("str:%s pos:%d\n", str, pos) ;
    }
    bool operator < (const data &a) const {
        if (strcmp (str, a.str) == 0) return pos < a.pos ;
        return strcmp (str, a.str) < 0 ;
    }
    bool operator <= (const data &a) const {
        if (strcmp (str, a.str) == 0) return pos <= a.pos ;
        return strcmp (str, a.str) < 0 ;
    }
    bool operator == (const data &a) const {
        return strcmp (str, a.str) == 0 && pos == a.pos ;
    }
} ;

#endif