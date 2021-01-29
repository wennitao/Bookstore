#ifndef _BPlusTree
#define _BPlusTree

#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>

#include "data.hpp"

using namespace std;

const int size = 30 ;

class BPlusTree {
private:
    
    struct node {
        bool is_leaf ;
        int keyCnt ;
        int par, son[size + 3] ;
        data key[size + 1] ;

        node () {
            is_leaf = 1; keyCnt = 0; par = -1 ;
            fill (son, son + size + 3, -1) ;
        }

        void print() {
            printf("is_leaf:%d cnt:%d par:%d\nson:", is_leaf, keyCnt, par) ;
            for (int i = 0; i <= size; i ++) printf("%d ", son[i]) ;
            cout << endl ;
            for (int i = 0; i < keyCnt; i ++) cout << key[i].str << " " << key[i].pos << endl ;
        }
    } ;

    fstream io ;

    char file_name[100] ;
    int root = -1, node_cnt = 0 ;
    const int node_size = sizeof (node), offset = 2 * sizeof (int) ;

public:
    BPlusTree (const char *file) {
        strcpy (file_name, file) ;
        fstream in (file, ios::in | ios::binary) ;
        if (!in.is_open()) {
            fstream out (file, ios::out | ios::binary) ;
            out.write (reinterpret_cast<char *>(&root), sizeof root) ;
            out.write (reinterpret_cast<char *>(&node_cnt), sizeof node_cnt) ;
            out.close() ;
        } else {
            in.read (reinterpret_cast<char *>(&root), sizeof root) ;
            in.read (reinterpret_cast<char *>(&node_cnt), sizeof node_cnt) ;
        }
        in.close() ;
        io.open (file, ios::in | ios::out | ios::binary) ;
    }

    ~BPlusTree () {
        io.close() ;
    }

    void print (int v) {
        node cur = disk_read (v) ;
        printf("pos:%d\n", v) ;
        cur.print() ;
        cout << endl ;
        for (int i = 0; i <= cur.keyCnt; i ++)
            if (cur.son[i] != -1) {
                print (cur.son[i]) ;
            }
    }

    void print () {
        print (root) ;
    }

    void clear (data &tmp) {
        memset (tmp.str, 0, sizeof tmp.str); tmp.pos = -1 ;
    }

    node disk_read (int pos) {
        io.seekg (pos + offset, ios::beg) ;
        node cur ;
        io.read (reinterpret_cast<char *>(&cur), sizeof (cur)) ;
        return cur ;
    }

    void disk_write (int pos, node &x) {
        io.seekp (pos + offset, ios::beg) ;
        io.write (reinterpret_cast<char *>(&x), sizeof (x)) ;
    }

    void update_root () {
        io.seekp (0, ios::beg) ;
        io.write (reinterpret_cast<char *>(&root), sizeof root) ;
        io.write (reinterpret_cast<char *>(&node_cnt), sizeof node_cnt) ;
    }

    pair<int, int> find (int v, const data &x) { //find node == x
        if  (v == -1) return make_pair (-1, -1) ;
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
        if (v == -1) return ;
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
                vector<int> tmp ;
                find (cur.son[pos], x, tmp) ;
                for (int i = 0; i < tmp.size(); i ++) res.push_back (tmp[i]) ;
                for (; pos < cur.keyCnt && strcmp (cur.key[pos].str, x.str) == 0; pos ++) {
                    tmp.clear() ;
                    find (cur.son[pos + 1], x, tmp) ;
                    for (int i = 0; i < tmp.size(); i ++) res.push_back (tmp[i]) ;
                }
            }
        }
    }

    pair<int, int> find (const data &x) {
        return find (root, x) ;
    }
    int findKey (const data &x) {
        pair<int, int> pos = find (x) ;
        if (pos.first == -1) return -1 ;
        node cur = disk_read (pos.first) ;
        return cur.key[pos.second].pos ;
    }

    void find (const data &x, vector<int> &res) {
        find (root, x, res) ;
    }

    int search (int v, const data &x) { //find the leaf_node where can insert x
        node cur = disk_read (v) ;
        if (cur.is_leaf) return v ;
        int pos = 0 ;
        for (; pos < cur.keyCnt && cur.key[pos] < x; pos ++) ;
        if (pos == cur.keyCnt || x < cur.key[pos])
            return search (cur.son[pos], x) ;
        else
            return search (cur.son[pos + 1], x) ;
    }

    void update_son_fa (int son, int fa) {
        node cur = disk_read (son) ;
        cur.par = fa ;
        disk_write (son, cur) ;
    }

    void insert (int &fa, int lchild, int rchild, const data &x) {
        if (fa == -1) {
            fa = (node_cnt ++) * node_size ;
            node par_node ;
            par_node.is_leaf = 0; par_node.key[par_node.keyCnt ++] = x ;
            par_node.son[0] = lchild; par_node.son[1] = rchild ;
            update_son_fa (lchild, fa); update_son_fa (rchild, fa) ;
            disk_write (fa, par_node) ;
            root = fa ;
        } else {
            node par_node = disk_read (fa) ;
            int pos = 0 ;
            for (; pos < par_node.keyCnt && par_node.key[pos] < x; pos ++) ;
            for (int i = par_node.keyCnt - 1; i >= pos; i --) par_node.key[i + 1] = par_node.key[i] ;
            par_node.key[pos] = x ;
            for (int i = par_node.keyCnt; i >= pos + 1; i --) par_node.son[i + 1] = par_node.son[i] ;
            par_node.son[pos] = lchild; par_node.son[pos + 1] = rchild ;
            update_son_fa (lchild, fa); update_son_fa (rchild, fa) ;
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
                    update_son_fa (par_node.son[i], nxt_node_pos) ;

                    par_node.son[i] = -1 ;
                }
                nxt_node.son[par_node.keyCnt - (size / 2 + 1)] = par_node.son[par_node.keyCnt] ;
                update_son_fa (par_node.son[par_node.keyCnt], nxt_node_pos) ;

                par_node.keyCnt = size / 2 ;
                data tmp = par_node.key[size / 2] ;
                clear (par_node.key[size / 2]) ;
                disk_write (fa, par_node); disk_write (nxt_node_pos, nxt_node) ;
                insert (par_node.par, fa, nxt_node_pos, tmp) ;
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

                disk_write (cur_pos, cur); disk_write (nxt_pos, nxt) ;
                insert (cur.par, cur_pos, nxt_pos, nxt.key[0]) ;
            }
        }
        update_root() ;
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
        if (son_pos + 1 <= par_node.keyCnt) rbro = par_node.son[son_pos + 1] ;
        node lbro_node, rbro_node ;
        if (lbro != -1) lbro_node = disk_read (lbro) ;
        if (rbro != -1) rbro_node = disk_read (rbro) ;

        if (lbro != -1 && lbro_node.keyCnt > size / 2) { //borrow one from left brother
            for (int i = cur.keyCnt; i >= 1; i --)
                cur.key[i] = cur.key[i - 1] ;
            for (int i = size + 1; i >= 1; i --)
                cur.son[i] = cur.son[i - 1] ;
            cur.key[0] = par_node.key[son_pos - 1] ;

            cur.son[0] = lbro_node.son[lbro_node.keyCnt] ;
            update_son_fa (lbro_node.son[lbro_node.keyCnt], v) ;

            cur.keyCnt ++ ;
            par_node.key[son_pos - 1] = lbro_node.key[lbro_node.keyCnt - 1] ;
            clear (lbro_node.key[lbro_node.keyCnt - 1]); lbro_node.keyCnt -- ;

            disk_write (v, cur) ;
            disk_write (lbro, lbro_node); disk_write (par, par_node) ;
        } else if (rbro != -1 && rbro_node.keyCnt > size / 2) { //borrow one from right brother
            cur.key[cur.keyCnt ++] = par_node.key[son_pos] ;
            
            cur.son[cur.keyCnt] = rbro_node.son[0] ;
            update_son_fa (rbro_node.son[0], v) ;

            par_node.key[son_pos] = rbro_node.key[0] ;

            for (int i = 1; i < rbro_node.keyCnt; i ++)
                rbro_node.key[i - 1] = rbro_node.key[i] ;
            for (int i = 1; i <= size + 2; i ++)
                rbro_node.son[i - 1] = rbro_node.son[i] ;
            clear (rbro_node.key[rbro_node.keyCnt - 1]); rbro_node.keyCnt -- ;

            disk_write (v, cur) ;
            disk_write (rbro, rbro_node); disk_write (par, par_node) ;
        } else {
            if (lbro != -1) { //merge with left brother
                lbro_node.key[lbro_node.keyCnt ++] = par_node.key[son_pos - 1] ;
                for (int i = son_pos; i < par_node.keyCnt; i ++)
                    par_node.key[i - 1] = par_node.key[i] ;
                clear (par_node.key[par_node.keyCnt - 1]); par_node.keyCnt -- ;

                for (int i = 0; i <= cur.keyCnt; i ++) {
                    lbro_node.son[lbro_node.keyCnt + i] = cur.son[i] ;
                    update_son_fa (cur.son[i], lbro) ;
                }
                for (int i = 0; i < cur.keyCnt; i ++)
                    lbro_node.key[lbro_node.keyCnt ++] = cur.key[i] ;
                for (int i = son_pos; i <= size + 2; i ++)
                    par_node.son[i - 1] = par_node.son[i] ;
                par_node.son[son_pos - 1] = lbro ;

                if (par_node.keyCnt == 0 && par == root) {
                    root = lbro; lbro_node.par = -1 ;
                    disk_write (lbro, lbro_node); 
                } else {
                    disk_write (lbro, lbro_node) ;
                    disk_write (par, par_node) ;
                    erase_par (par) ;
                }
            } else if (rbro != -1) { //merge with right brother
                cur.key[cur.keyCnt ++] = par_node.key[son_pos] ;
                for (int i = son_pos + 1; i < par_node.keyCnt; i ++)
                    par_node.key[i - 1] = par_node.key[i] ;
                clear (par_node.key[par_node.keyCnt - 1]); par_node.keyCnt -- ;

                for (int i = 0; i <= rbro_node.keyCnt; i ++) {
                    cur.son[cur.keyCnt + i] = rbro_node.son[i] ;
                    update_son_fa (rbro_node.son[i], v) ;
                }
                for (int i = 0; i < rbro_node.keyCnt; i ++)
                    cur.key[cur.keyCnt ++] = rbro_node.key[i] ;
                for (int i = son_pos + 1; i <= size + 2; i ++)
                    par_node.son[i - 1] = par_node.son[i] ;
                par_node.son[son_pos] = v ;

                if (par_node.keyCnt == 0 && par == root) {
                    root = v; cur.par = -1 ;
                    disk_write (v, cur);
                } else {
                    disk_write (v, cur);
                    disk_write (par, par_node) ;
                    erase_par (par) ;
                }
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
        if (cur.keyCnt >= size / 2) { //valid
            disk_write (pos.first, cur) ;
        } else {
            if (cur.par == -1) { //have no father
                disk_write (pos.first, cur) ;
                return ;
            }
            int par = cur.par ;
            node par_node = disk_read (par) ;
            int son_pos = 0 ;
            for (; par_node.son[son_pos] != pos.first; son_pos ++) ;
            int lbro = -1, rbro = -1 ;
            if (son_pos > 0) lbro = par_node.son[son_pos - 1] ;
            if (son_pos + 1 <= par_node.keyCnt) rbro = par_node.son[son_pos + 1] ;
            
            node lbro_node, rbro_node ;
            if (lbro != -1) lbro_node = disk_read (lbro) ;
            if (rbro != -1) rbro_node = disk_read (rbro) ;

            if (lbro != -1 && lbro_node.keyCnt > size / 2) { //borrow one from left brother
                for (int i = cur.keyCnt; i >= 1; i --)
                    cur.key[i] = cur.key[i - 1] ;
                cur.key[0] = lbro_node.key[lbro_node.keyCnt - 1] ;
                clear (lbro_node.key[lbro_node.keyCnt - 1]); lbro_node.keyCnt -- ;
                cur.keyCnt ++ ;
                par_node.key[son_pos - 1] = cur.key[0] ;

                disk_write (pos.first, cur) ;
                disk_write (lbro, lbro_node); disk_write (cur.par, par_node) ;
            } else if (rbro != -1 && rbro_node.keyCnt > size / 2) { //borrow one from right brother
                cur.key[cur.keyCnt ++] = rbro_node.key[0] ;
                for (int i = 1; i < rbro_node.keyCnt; i ++)
                    rbro_node.key[i - 1] = rbro_node.key[i] ;
                clear (rbro_node.key[rbro_node.keyCnt - 1]); rbro_node.keyCnt -- ;
                par_node.key[son_pos] = rbro_node.key[0] ;

                disk_write (pos.first, cur) ;
                disk_write (rbro, rbro_node); disk_write (cur.par, par_node) ;
            } else {
                if (lbro != -1) { //merge with left brother
                    for (int i = 0; i < cur.keyCnt; i ++)
                        lbro_node.key[lbro_node.keyCnt ++] = cur.key[i] ;

                    for (int i = son_pos + 1; i <= size + 2; i ++)
                        par_node.son[i - 1] = par_node.son[i] ;
                    par_node.son[son_pos - 1] = lbro; par_node.son[par_node.keyCnt] = -1 ;
                    if (son_pos < par_node.keyCnt) {
                        for (int i = son_pos; i < par_node.keyCnt; i ++)
                            par_node.key[i - 1] = par_node.key[i] ;
                    }
                    par_node.keyCnt -- ;
                    if (par_node.keyCnt == 0 && par == root) {
                        root = lbro; lbro_node.par = -1 ;
                        disk_write (lbro, lbro_node); 
                    } else {
                        disk_write (lbro, lbro_node); 
                        disk_write (cur.par, par_node) ;
                        erase_par (cur.par) ;
                    }
                } else if (rbro != -1) { //merge with right brother
                    for (int i = 0; i < rbro_node.keyCnt; i ++)
                        cur.key[cur.keyCnt ++] = rbro_node.key[i] ;
                    for (int i = son_pos + 2; i <= size + 2; i ++)
                        par_node.son[i - 1] = par_node.son[i] ;
                    par_node.son[son_pos] = pos.first; par_node.son[par_node.keyCnt] = -1 ;
                    if (son_pos + 1 < par_node.keyCnt) {
                        for (int i = son_pos + 1; i < par_node.keyCnt; i ++)
                            par_node.key[i - 1] = par_node.key[i] ;
                    }
                    par_node.keyCnt -- ;

                    if (par_node.keyCnt == 0 && par == root) {
                        root = pos.first; cur.par = -1 ;
                        disk_write (pos.first, cur); 
                    } else {
                        disk_write (pos.first, cur); 
                        disk_write (cur.par, par_node) ;
                        erase_par (cur.par) ;
                    }
                }
            }
        }
        update_root() ;
    }
} ;

#endif
