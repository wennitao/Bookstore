#ifndef _BPlusTree
#define _BPlusTree

#include <iostream>
#include <cstdio>
#include <fstream>

#include "data.hpp"

using namespace std;

const int size = 4 ;

class BPlusTree {
private:
    
    struct node {
        bool is_leaf ;
        int keyCnt ;
        int par, son[size + 2] ;
        data key[size + 1] ;

        node () {
            is_leaf = 1; keyCnt = 0; par = -1 ;
            fill (son, son + size + 2, -1) ;
        }

        void print() {
            printf("is_leaf:%d cnt:%d\npar:%d\nson:", is_leaf, keyCnt, par) ;
            for (int i = 0; i <= size; i ++) printf("%d ", son[i]) ;
            cout << endl ;
            for (int i = 0; i < size; i ++) cout << key[i].str << " " << key[i].pos << endl ;
        }
    } ;

    char file_name[100] ;
    int root = -1, node_cnt = 0 ;
    const int node_size = sizeof (node) ;

public:
    BPlusTree (const char *file) {
        strcpy (file_name, file) ;
        fstream in (file, ios::in | ios::binary) ;
        if (!in.is_open()) {
            fstream out (file, ios::out | ios::binary) ;
            out.close() ;
        }
        in.close() ;
    }

    void print (int v) {
        node cur = disk_read (v) ;
        cur.print() ;
        cout << endl ;
        for (int i = 0; i <= size; i ++)
            if (cur.son[i] != -1) {
                printf("pos:%d\n", cur.son[i]) ;
                print (cur.son[i]) ;
            }
    }

    void print () {
        print (root) ;
    }

    void clear (data &tmp) {
        memset (tmp.str, 0, sizeof tmp.str); tmp.pos = -1 ;
    }

    /*void update (pair<int, int> pos, const data &x) {
        node cur = disk_read (pos.first) ;
        cur.key[pos.second] = x ;
        disk_write (pos.first, cur) ;
    }*/

    node disk_read (int pos) {
        fstream in ;
        in.open (file_name, ios::in | ios::binary) ;
        in.seekg (pos, ios::beg) ;
        node cur ;
        in.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
        in.close() ;
        return cur ;
    }

    void disk_write (int pos, node &x) {
        fstream out ;
        out.open (file_name, ios::in | ios::out | ios::binary) ;
        out.seekp (pos, ios::beg) ;
        out.write (reinterpret_cast<char *>(&x), sizeof (x)) ;
        out.close() ;
    }

    pair<int, int> find (int v, const data &x) { //find node == x
        //printf("v:%d\n", v) ;
        node cur = disk_read (v) ;
        int pos = 0 ;
        for (; pos < cur.keyCnt && cur.key[pos] < x; pos ++) ;
        if (cur.is_leaf) {
            if (pos < cur.keyCnt && cur.key[pos] == x) return make_pair (v, pos) ;
            else return make_pair (-1, -1) ;
        } else {
            if (pos == cur.keyCnt || x < cur.key[pos]) return find (cur.son[pos], x) ;
            else return find (cur.son[pos + 1], x) ;
        }
    }

    void find (int v, const data &x, vector<int> &res) { //find node == x
        //printf("v:%d\n", v) ;
        node cur = disk_read (v) ;
        int pos = 0 ;
        for (; pos < cur.keyCnt && cur.key[pos] < x; pos ++) ;
        if (cur.is_leaf) {
            for (int i = pos; i < cur.keyCnt; i ++) {
                if (strcmp (cur.key[i].str, x.str) == 0) res.push_back (cur.key[i].pos) ;
                else return ;
            }
        } else {
            if (pos == cur.keyCnt || strcmp (x.str, cur.key[pos].str) < 0) find (cur.son[pos], x, res) ;
            else {
                for (; pos < cur.keyCnt && strcmp (cur.key[pos].str, x.str) == 0; pos ++) {
                    vector<int> tmp ;
                    find (cur.son[pos + 1], x, tmp) ;
                    for (int i = 0; i < tmp.size(); i ++) res.push_back (tmp[i]) ;
                }
            }
        }
    }

    pair<int, int> find (const data &x) {
        return find (root, x) ;
    }

    void find (const data &x, vector<int> &res) {
        find (root, x, res) ;
    }

    /*T find (const pair<int, int> pos) {
        node cur = disk_read (pos.first) ;
        return cur.key[pos.second] ;
    }

    T findKey (const T &x) {
        pair<int, int> pos = find (x) ;
        if (pos.first == -1) throw "not found" ;
        node cur = disk_read (pos.first) ;
        return cur.key[pos.second] ;
    }*/

    int search (int v, const data &x) { //find the leaf_node where can insert x
        node cur = disk_read (v) ;
        if (cur.is_leaf) return v ;
        int pos = 0 ;
        for (; pos < cur.keyCnt && cur.key[pos] < x; pos ++) ;
        return search (cur.son[pos], x) ;
    }

    void insert (int &fa, int lchild, int rchild, const data &x) {
        if (fa == -1) {
            fa = (node_cnt ++) * node_size ;
            node par_node ;
            par_node.is_leaf = 0; par_node.key[par_node.keyCnt ++] = x ;
            par_node.son[0] = lchild; par_node.son[1] = rchild ;
            disk_write (fa, par_node) ;
            root = fa ;
            return ;
        } else {
            node par_node = disk_read (fa) ;
            int pos = 0 ;
            for (; pos < par_node.keyCnt && par_node.key[pos] < x; pos ++) ;
            for (int i = par_node.keyCnt - 1; i >= pos; i --) par_node.key[i + 1] = par_node.key[i] ;
            par_node.key[pos] = x ;
            for (int i = par_node.keyCnt; i >= pos + 1; i --) par_node.son[i + 1] = par_node.son[i] ;
            par_node.son[pos] = lchild; par_node.son[pos + 1] = rchild ;
            par_node.keyCnt ++ ;
            if (par_node.keyCnt <= size) {
                disk_write (fa, par_node) ;
            } else {
                node nxt_node ;
                nxt_node.is_leaf = 0 ;
                int nxt_node_pos = (node_cnt ++) * node_size ;
                for (int i = size / 2 + 1; i < par_node.keyCnt; i ++) {
                    nxt_node.key[nxt_node.keyCnt ++] = par_node.key[i] ;
                    clear (par_node.key[i]) ;
                    nxt_node.son[i - (size / 2 + 1)] = par_node.son[i] ;
                    par_node.son[i] = -1 ;
                }
                nxt_node.son[par_node.keyCnt - (size / 2 + 1)] = par_node.son[par_node.keyCnt] ;
                insert (par_node.par, fa, nxt_node_pos, par_node.key[size / 2]) ;
                par_node.keyCnt = size / 2 ;
                clear (par_node.key[size / 2]) ;
                nxt_node.par = par_node.par ;
                disk_write (fa, par_node); disk_write (nxt_node_pos, nxt_node) ;
            }
        }
    }

    void insert (const data &x) {
        if (root == -1) {
            node cur ;
            cur.key[cur.keyCnt ++] = x ;
            root = 0; node_cnt ++ ;
            disk_write (0, cur) ;
        } else {
            int cur_pos = search (root, x) ;
            node cur = disk_read (cur_pos) ;
            cur.key[cur.keyCnt ++] = x ;
            sort (cur.key, cur.key + cur.keyCnt) ;
            if (cur.keyCnt <= size) {
                disk_write (cur_pos, cur) ;
            } else {
                node nxt ;
                int nxt_pos = (node_cnt ++) * node_size ;
                for (int i = size / 2; i < cur.keyCnt; i ++) {
                    nxt.key[nxt.keyCnt ++] = cur.key[i] ;
                    clear (cur.key[i]) ;
                }
                cur.keyCnt = size / 2 ;
                insert (cur.par, cur_pos, nxt_pos, nxt.key[0]) ;
                nxt.par = cur.par ;
                disk_write (cur_pos, cur); disk_write (nxt_pos, nxt) ;
            }
        }
    }

    void erase_par (int v) {
        node cur = disk_read (v) ;
        if (cur.keyCnt >= size / 2) return ;
        int par = cur.par ;
        if (par == -1) return ;
        node par_node = disk_read (par) ;
        int son_pos = 0 ;
        for (; par_node.son[son_pos] != v; son_pos ++) ;

        int lbro = -1, rbro = -1 ;
        if (son_pos > 0) lbro = par_node.son[son_pos - 1] ;
        if (par_node.son[son_pos + 1] != -1) rbro = par_node.son[son_pos + 1] ;
        node lbro_node, rbro_node ;
        if (lbro != -1) lbro_node = disk_read (lbro) ;
        if (rbro != -1) rbro_node = disk_read (rbro) ;

        if (lbro != -1 && lbro_node.keyCnt > size / 2) {
            for (int i = cur.keyCnt; i >= 1; i --)
                cur.key[i] = cur.key[i - 1] ;
            cur.key[0] = par_node.key[son_pos - 1] ;
            cur.keyCnt ++ ;
            
            par_node.key[son_pos - 1] = lbro_node.key[lbro_node.keyCnt - 1] ;
            clear (lbro_node.key[lbro_node.keyCnt - 1]); lbro_node.keyCnt -- ;
            lbro_node.keyCnt -- ;

            disk_write (v, cur) ;
            disk_write (lbro, lbro_node); disk_write (par, par_node) ;
        } else if (rbro != -1 && rbro_node.keyCnt > size / 2) {
            cur.key[cur.keyCnt ++] = par_node.key[son_pos] ;
            par_node.key[son_pos] = rbro_node.key[0] ;

            for (int i = 1; i < rbro_node.keyCnt; i ++)
                rbro_node.key[i - 1] = rbro_node.key[i] ;
            clear (rbro_node.key[rbro_node.keyCnt - 1]); rbro_node.keyCnt -- ;

            disk_write (v, cur) ;
            disk_write (rbro, rbro_node); disk_write (par, par_node) ;
        } else {
            if (lbro != -1) {
                lbro_node.key[lbro_node.keyCnt ++] = par_node.key[son_pos - 1] ;
                for (int i = son_pos; i < par_node.keyCnt; i ++)
                    par_node.key[i - 1] = par_node.key[i] ;
                clear (par_node.key[par_node.keyCnt - 1]); par_node.keyCnt -- ;

                for (int i = 0; i < cur.keyCnt; i ++)
                    lbro_node.key[lbro_node.keyCnt ++] = cur.key[i] ;
                for (int i = son_pos + 1; i <= size + 1; i ++)
                    par_node.son[i - 1] = par_node.son[i] ;

                disk_write (lbro, lbro_node); disk_write (par, par_node) ;
                erase_par (par) ;
            } else if (rbro != -1) {
                cur.key[cur.keyCnt ++] = par_node.key[son_pos] ;
                for (int i = son_pos + 1; i < par_node.keyCnt; i ++)
                    par_node.key[i - 1] = par_node.key[i] ;
                clear (par_node.key[par_node.keyCnt - 1]); par_node.keyCnt -- ;

                for (int i = 0; i < rbro_node.keyCnt; i ++)
                    cur.key[cur.keyCnt ++] = rbro_node.key[i] ;
                for (int i = son_pos + 2; i <= size + 1; i ++)
                    par_node.son[i - 1] = par_node.son[i] ;
                
                disk_write (v, cur); disk_write (par, par_node) ;
                erase_par (par) ;
            }
        }
    }

    void erase (const data &x) {
        pair<int, int> pos = find (x) ;
        if (pos.first == -1) throw "not found" ;
        node cur = disk_read (pos.first) ;
        for (int i = pos.second + 1; i < cur.keyCnt; i ++)
            cur.key[i - 1] = cur.key[i] ;
        clear (cur.key[cur.keyCnt - 1]); cur.keyCnt -- ;
        if (cur.keyCnt >= size / 2) {
            disk_write (pos.first, cur) ;
        } else {
            if (cur.par == -1) {
                disk_write (pos.first, cur) ;
                return ;
            }
            node par = disk_read (cur.par) ;
            int son_pos = 0 ;
            for (; par.son[son_pos] != pos.first; son_pos ++) ;
            int lbro = -1, rbro = -1 ;
            if (son_pos > 0) lbro = par.son[son_pos - 1] ;
            if (par.son[son_pos + 1] != -1) rbro = par.son[son_pos + 1] ;
            
            node lbro_node, rbro_node ;
            if (lbro != -1) lbro_node = disk_read (lbro) ;
            if (rbro != -1) rbro_node = disk_read (rbro) ;

            if (lbro != -1 && lbro_node.keyCnt > size / 2) {
                for (int i = cur.keyCnt; i >= 1; i --)
                    cur.key[i] = cur.key[i - 1] ;
                cur.key[0] = lbro_node.key[lbro_node.keyCnt - 1] ;
                clear (lbro_node.key[(lbro_node.keyCnt --) - 1]) ;
                cur.keyCnt ++ ;
                par.key[son_pos - 1] = cur.key[0] ;

                disk_write (pos.first, cur) ;
                disk_write (lbro, lbro_node); disk_write (cur.par, par) ;
            } else if (rbro != -1 && rbro_node.keyCnt > size / 2) {
                cur.key[cur.keyCnt ++] = rbro_node.key[0] ;
                for (int i = 1; i < rbro_node.keyCnt; i ++)
                    rbro_node.key[i - 1] = rbro_node.key[i] ;
                clear (rbro_node.key[(rbro_node.keyCnt --) - 1]) ;
                par.key[son_pos] = cur.key[cur.keyCnt - 1] ;

                disk_write (pos.first, cur) ;
                disk_write (rbro, rbro_node); disk_write (cur.par, par) ;
            } else {
                if (lbro != -1) {
                    for (int i = 0; i < cur.keyCnt; i ++)
                        lbro_node.key[lbro_node.keyCnt ++] = cur.key[i] ;

                    for (int i = son_pos + 1; i <= size + 1; i ++)
                        par.son[i - 1] = par.son[i] ;
                    for (int i = son_pos; i < par.keyCnt; i ++)
                        par.key[i - 1] = par.key[i] ;
                    par.keyCnt -- ;
                    if (par.keyCnt == 0) {
                        root = lbro ;
                        lbro_node.par = -1 ;
                    }
                    
                    disk_write (lbro, lbro_node); 
                    if (par.keyCnt) {
                        disk_write (cur.par, par) ;
                        erase_par (cur.par) ;
                    }
                } else if (rbro != -1) {
                    for (int i = 0; i < rbro_node.keyCnt; i ++)
                        cur.key[cur.keyCnt ++] = rbro_node.key[i] ;
                    for (int i = son_pos + 2; i <= size + 1; i ++)
                        par.son[i - 1] = par.son[i] ;
                    for (int i = son_pos + 1; i < par.keyCnt; i ++)
                        par.key[i - 1] = par.key[i] ;
                    par.keyCnt -- ;
                    if (par.keyCnt == 0) {
                        root = pos.first ;
                        cur.par = -1 ;
                    }
                    
                    disk_write (pos.first, cur); 
                    if (par.keyCnt) {
                        disk_write (cur.par, par) ;
                        erase_par (cur.par) ;
                    }
                }
            }
        }
    }
} ;

#endif
