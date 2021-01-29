#include <bits/stdc++.h>
#include "src/data.hpp"
#include "src/B+Tree.hpp"
using namespace std;
int main() {
    //for (int t = 0; ; t ++) {
        //printf("Test %d\n", t) ;
        system ("rm *.dat") ;
        BPlusTree test ("test.dat") ;
        int seed = time (0) ;
        //int seed = 1611837369 ;
        srand (seed) ;
        char s[100010][20] ;
        memset (s, 0, sizeof s) ;
        int n = 100000 ;
        for (int i = 1; i <= n; i ++) {
            int len = 1 + rand() % 10 ;
            for (int j = 0; j < len; j ++)
                s[i][j] = 'a' + rand() % 26 ;
            test.insert (data (s[i], i)) ;
        }
        for (int i = 1; i <= n; i ++) {
            test.erase (data  (s[i], i)) ;
            if (i == n) break ;
            int j = rand() % (n - i) + i + 1;
            int res = test.findKey (data (s[j], j)) ;
            if (j != res) {
                printf("seed:%d test:(%s %d) %d\n", seed, s[j], j, res) ;
                puts ("QAQ") ;
                return 0 ;
            }
        }
    //}
    //int seed = 1611818624 ;*/
    return 0 ;
}