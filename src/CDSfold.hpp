#include <algorithm> // only to use fill
#include <fstream>
#include <map>
//#include <iostream>
//#include <stdlib.h>
#include "codon.hpp"
using namespace std;

#pragma once

inline auto E_hairpin(int size, int type, int si1, int sj1, const char *string, paramT *P) -> int;
inline auto E_intloop(int n1, int n2, int type, int type_2, int si1, int sj1, int sp1, int sq1, paramT *P) -> int;

using stack = struct stack {
    int i;
    int j;
    int Li;
    int Rj;
    int ml;
};

using bond = struct bond {
    int i;
    int j;
};

inline auto TermAU(int const &type, paramT *const &P) -> int;

auto getMatrixSize(int len, int w) -> int {
    int size = 0;
    for (int i = 1; i <= w; i++) {
        size += len - (i - 1); // マトリクスの斜めの要素数を足し合わせるイメージ
                               // i=1のときは、対角要素（len個）を足し合わせる。
    }

    cout << "The size of matrix is " << size << endl;
    return size;
}

inline auto getIndx(int const &i, int const &j, int const &w, int *const &indx) -> int {
    return indx[j] + i - MAX2(0, j - w); // j-wは使わない要素の数。
                                         // wが指定されない(=length)と、j列にはj個分(1<i<j)の要素が用意される。
                                         // wが指定されると、j列にはで使う要素はw個、使わない要素はj-w個となる。
}

auto predict_memory(int len, int w, vector<vector<int>> &pos2nuc) -> float {
    // int size = getMatrixSize(len, w);
    long int total_bytes = 0;
    // int n_test;
    //	int limit = LONG_MAX;
    for (int i = 1; i <= len; i++) {
        for (int j = i; j <= MIN2(len, i + w - 1); j++) {
            // cout << i << "," << j << endl;
            total_bytes += sizeof(int *) * (pos2nuc[i].size() + 4) * 2; // +4 is an empirical value
            for (unsigned int L = 0; L < pos2nuc[i].size(); L++) {
                total_bytes += sizeof(int) * (pos2nuc[j].size() + 4) * 2; // +4 is an empirical value
                // n_test++;
                if (total_bytes >= LONG_MAX - 10000)
                    return (float)LONG_MAX / (1024 * 1024);
            }
        }
    }
    // cout << n_test << endl;
    // cout << total_bytes << endl;
    total_bytes *= 1.2; // 1.2 is an empirical value
    if (total_bytes >= LONG_MAX - 10000)
        return (float)LONG_MAX / (1024 * 1024);

    return (float)total_bytes / (1024 * 1024);
}

void clear_sec_bp(stack *s, bond *b, int len) {
    for (int i = 0; i < 500; i++) {
        s[i].i = -INF;
        s[i].j = -INF;
        s[i].Li = -INF;
        s[i].Rj = -INF;
        s[i].ml = -INF;
    }
    for (int i = 0; i < len / 2; i++) {
        b[i].i = -INF;
        b[i].j = -INF;
    }
}

void allocate_arrays(
    int len,
    int *indx,
    int w,
    vector<vector<int>> &pos2nuc,
    vector<vector<vector<int>>> & c,
    vector<vector<vector<int>>> & m,
    vector<vector<vector<int>>> & f,
    vector<array<array<int, 4>, 4>> & dml,
    vector<array<array<int, 4>, 4>> & dml1,
    vector<array<array<int, 4>, 4>> & dml2,
    vector<int> & chkc,
    vector<int> & chkm,
    vector<bond> & b
) {
    int size = getMatrixSize(len, w);
    //	int n_elm = 0;
    int total_bytes = 0;
    // int test_bytes = 0;

    //	*c   = new int**[len*(len+1)/2+1];
    //	*m   = new int**[len*(len+1)/2+1];
    c.resize(size + 1);
    m.resize(size + 1);
    //	*f2   = new int**[size+1];
    for (int i = 1; i <= len; i++) {
        for (int j = i; j <= MIN2(len, i + w - 1); j++) {
            // cout << i << " " << j << endl;
            // int ij = indx[j]+i;
            int ij = getIndx(i, j, w, indx);
            c[ij].resize(pos2nuc[i].size());
            m[ij].resize(pos2nuc[i].size());
            //			(*f2)[ij]   = new int*[pos2nuc[i].size()];
            // total_bytes += sizeof(int*) * (pos2nuc[i].size()+1) * 2 * 2; // *2 is empirical constant
            total_bytes += sizeof(int *) * (pos2nuc[i].size() + 4) * 2; // +4 is an empirical value
            for (unsigned int L = 0; L < pos2nuc[i].size(); L++) {
                c[ij][L].resize(pos2nuc[j].size());
                m[ij][L].resize(pos2nuc[j].size());
                //				(*f2)[ij][L]   = new int[pos2nuc[j].size()];
                // total_bytes += sizeof(int) * (pos2nuc[j].size()+1) * 2 * 2; // *2 is empirical constant
                //				n_elm += (pos2nuc[j].size()+1);
                total_bytes += sizeof(int) * (pos2nuc[j].size() + 4) * 2; // +4 is an empirical value
            }
        }
    }

    total_bytes *= 1.2; // 1.2 is an empirical value
                        //	cout << "sizeof(int): " << sizeof(int) << endl;
                        //	cout << "sizeof(int*): " << sizeof(int***) << endl;
                        //	cout << "N_ELM: "<< n_elm << endl;
                        //	cout << "test_bytes: "<< (float)test_bytes/(1024*1024) << endl;
                        //	return test_bytes;

    //	int total_bytes = n_elm * sizeof(int) * 2; // *2 is m and c
    //	float total_Mb = (float)total_bytes/(1024*1024);
    //	cout << "estimated memory size: " << total_Mb << " Mb" << endl;

    f.resize(len + 1);
    dml.resize(len + 1, {{{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF}}});
    dml1.resize(len + 1, {{{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF}}});
    dml2.resize(len + 1, {{{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF},{INF, INF, INF, INF}}});
    for (int j = 1; j <= len; j++) {
        f[j].resize(pos2nuc[j].size());
        for (unsigned int L = 0; L < pos2nuc[1].size(); L++) { // The first position
            f[j][L].resize(pos2nuc[j].size());
        }

        // for (unsigned int L = 0; L < 4; L++) {
        //     (*dml)[j][L] = new int[4];  // always secure 4 elements, because the maximum number of nucleotides is 4
        //     (*dml1)[j][L] = new int[4]; // always secure 4 elements, because the maximum number of nucleotides is 4
        //     (*dml2)[j][L] = new int[4]; // always secure 4 elements, because the maximum number of nucleotides is 4
        //     fill((*dml)[j][L], (*dml)[j][L] + 4, INF);
        //     fill((*dml1)[j][L], (*dml1)[j][L] + 4, INF);
        //     fill((*dml2)[j][L], (*dml2)[j][L] + 4, INF);
        // }
    }

    //	*chkc   = new int[len*(len+1)/2+1];
    //	*chkm   = new int[len*(len+1)/2+1];

    //	fill(*chkc, *chkc+len*(len+1)/2, INF);
    //	fill(*chkm, *chkm+len*(len+1)/2, INF);

    chkc.resize(size + 1, INF);
    chkm.resize(size + 1, INF);

    b.resize(len / 2);

    //	return (float)total_bytes/(1024*1024) * 1.25; // 1.2 is empirical constant
    // return (float)total_bytes/(1024*1024);
}

void allocate_F2(int len, int *indx, int w, vector<vector<int>> &pos2nuc, vector<vector<vector<int>>> & f2) {
    int size = getMatrixSize(len, w);
    f2.resize(size + 1);
    for (int i = 1; i <= len; i++) {
        for (int j = i; j <= MIN2(len, i + w - 1); j++) {
            int ij = getIndx(i, j, w, indx);
            f2[ij].resize(pos2nuc[i].size());
            for (unsigned int L = 0; L < pos2nuc[i].size(); L++) {
                f2[ij][L].resize(pos2nuc[j].size());
            }
        }
    }
}

void set_ij_indx(int *a, int length) {
    for (int n = 1; n <= length; n++) {
        a[n] = (n * (n - 1)) / 2;
    }
}

void set_ij_indx(int *a, int length, int w) {
    if (w <= 0) {
        cerr << "Invalid w:" << w << endl;
        exit(1);
    }
    w = MIN2(length, w);
    int cum = 0;
    for (int n = 1; n <= length; n++) {
        a[n] = cum;
        // cout << n << ":" << a[n] << endl;
        if (n < w) {
            cum += n;
        } else {
            cum += w;
        }
    }
}

void set_arrays(int **a, int length) {
    *a = new int[length + 1]; //

    for (int n = 1; n <= length; n++) {
        (*a)[n] = (n * (n - 1)) / 2;
        // cout << *a[n] << endl;
    }
}

void make_i2r(int *n) {
    n[1] = 1;
    n[2] = 2;
    n[3] = 3;
    n[4] = 4;
    n[5] = 4; // 2nd position of L is converted to U
    n[6] = 4;
    n[7] = 3; // 2nd position of R is converted to G
    n[8] = 3;
}

void make_ii2r(int *n) {
    int s = 1;
    for (int i1 = 1; i1 <= 8; i1++) {
        for (int i2 = 1; i2 <= 8; i2++) {
            n[i1 * 10 + i2] = s++;
        }
    }
}

map<char, int> make_n2i() {
    map<char, int> m;
    m['A'] = 1;
    m['C'] = 2;
    m['G'] = 3;
    m['U'] = 4;
    m['V'] = 5; // 2nd position of L, before A/G
    m['W'] = 6; // 2nd position of L, before U/C
    m['X'] = 7; // 2nd position of R, before A/G
    m['Y'] = 8; // 2nd position of R, before U/C

    return m;
}

// void view_n2i(map<char, int> n2i, char const &c){
auto view_n2i(map<char, int> n2i, char c) -> int {
    // cout << c << endl;
    return n2i[c];
}

void make_i2n(char *n) {
    n[0] = ' ';
    n[1] = 'A';
    n[2] = 'C';
    n[3] = 'G';
    n[4] = 'U';
    n[5] = 'V';
    n[6] = 'W';
    n[7] = 'X';
    n[8] = 'Y';
}

void showPos2Nuc(vector<vector<int>> &v) {

    for (unsigned int i = 1; i <= v.size(); i++) {
        cout << i;
        for (unsigned int j = 0; j < v[i].size(); j++) {
            cout << "\t" << v[i][j];
        }
        cout << endl;
    }
}

void showPos2Nuc(vector<vector<int>> &v, char i2n[]) {

    for (unsigned int i = 1; i < v.size(); i++) {
        //	  for(unsigned int i = 1; i <= v.size(); i++){ // This is segmentaion fault
        cout << i;
        for (unsigned int j = 0; j < v[i].size(); j++) {
            cout << "\t" << i2n[v[i][j]];
        }
        cout << endl;
    }
}

vector<vector<int>> getPossibleNucleotide(char *aaseq, int aalen, codon &codon_table, map<char, int> &n2i,
                                          string excludedCodons) {

    vector<vector<int>> v;

    int nuclen = aalen * 3;
    v.resize(nuclen + 1);

    for (int i = 0; i < aalen; i++) {
        int nucpos = i * 3 + 1;
        char aa = aaseq[i];
        // cout << "test: " << aa << endl;
        // AMW TODO: should get codons from the codon table, filtering out
        // exclusions, *once* rather than generating new vector<string>s for each
        // aa position. This function is only called once, but it's still silly.
        vector<string> codons = codon_table.getCodons(aa, excludedCodons);
        for (int k = 0; k < 3; k++) { // for each codon position
            nucpos += k;

            if (aa == 'L' && k == 1) {
                v[nucpos].push_back(n2i['V']);
                v[nucpos].push_back(n2i['W']);
            } else if (aa == 'R' && k == 1) {
                v[nucpos].push_back(n2i['X']);
                v[nucpos].push_back(n2i['Y']);
            } else {
                bool flg_A, flg_C, flg_G, flg_U;
                flg_A = flg_C = flg_G = flg_U = false;

                for (unsigned int j = 0; j < codons.size(); j++) { // for each codon corresp. to each aa
                    char nuc = codons[j][k];
                    if (nuc == 'A' && flg_A == 0) {
                        v[nucpos].push_back(n2i[nuc]);
                        flg_A = 1;
                    } else if (nuc == 'C' && flg_C == 0) {
                        v[nucpos].push_back(n2i[nuc]);
                        flg_C = 1;
                    } else if (nuc == 'G' && flg_G == 0) {
                        v[nucpos].push_back(n2i[nuc]);
                        flg_G = 1;
                    } else if (nuc == 'U' && flg_U == 0) {
                        v[nucpos].push_back(n2i[nuc]);
                        flg_U = 1;
                    }
                }
            }
            nucpos -= k;
        }
    }
    return v;
}

void showChkMatrix(int *&m, int *&indx, int len, int w) {
    printf("REF:");
    for (int j = 1; j <= len; j++) {
        printf("\t%d", j);
    }
    printf("\n");

    for (int i = 1; i <= len; i++) {
        printf("REF:");
        printf("%d", i);
        for (int j = 1; j <= len; j++) {
            if (i >= j) {
                printf("\t-");
            } else {
                // printf("\t%d", m[indx[j]+i]);
                printf("\t%d", m[getIndx(i, j, w, indx)]);
            }
        }
        printf("\n");
    }
}

void showFixedMatrix(const int *m, int *indx, const int len, const int w) {
    printf("REF:");
    for (int j = 1; j <= len; j++) {
        printf("\t%d", j);
    }
    printf("\n");

    for (int i = 1; i <= len; i++) {
        printf("REF:");
        printf("%d", i);
        for (int j = 1; j <= len; j++) {
            if (i >= j) {
                printf("\t-");
            } else {
                // printf("\t%d", m[indx[j]+i]);
                printf("\t%d", m[getIndx(i, j, w, indx)]);
            }
        }
        printf("\n");
    }
}

vector<int> createNucConstraint(const char *s, int &len, map<char, int> &n2i) {
    vector<int> v;
    v.resize(len + 1);
    for (int i = 1; i <= len; i++) {
        v[i] = n2i[s[i]];
    }
    return v;
}

inline void InitRand() { srand((unsigned int)time(nullptr)); }

void shuffleStr(vector<string>(*ary), int size) {
    for (int i = 0; i < size; i++) {
        int j = rand() % size;
        string t = (*ary)[i];
        (*ary)[i] = (*ary)[j];
        (*ary)[j] = t;
    }
}
void shuffle(int ary[], int size) {
    for (int i = 0; i < size; i++) {
        int j = rand() % size;
        int t = ary[i];
        ary[i] = ary[j];
        ary[j] = t;
    }
}
vector<pair<int, int>> shufflePair(vector<pair<int, int>> ary, int size) {
    for (int i = 0; i < size; i++) {
        int j = rand() % size;
        // cout << rand() << ":" << j << endl;
        pair<int, int> t = ary[i];
        ary[i] = ary[j];
        ary[j] = t;
    }

    return ary;
}

void backtrack(string *optseq, stack *sector, vector<bond> & base_pair, vector<vector<vector<int>>> const &c, vector<vector<vector<int>>> const &m, vector<vector<vector<int>>> const &f2,
               int *const indx, const int &initL, const int &initR, paramT *const &P, const vector<int> &NucConst,
               const vector<vector<int>> &pos2nuc, const int &NCflg, int *const &i2r, int const &length, int const &w,
               int const (&BP_pair)[5][5], char *const &i2n, int *const &rtype, int *const &ii2r,
               vector<vector<int>> &Dep1, vector<vector<int>> &Dep2, int &DEPflg,
               vector<vector<vector<vector<pair<int, string>>>>> &, map<string, int> &predefE,
               vector<vector<vector<string>>> &substr, map<char, int> &n2i, const char *nucdef);

void backtrack2(string *optseq, stack *sector, vector<bond> & base_pair, vector<vector<vector<int>>> const &c, vector<vector<vector<int>>> const &m, vector<vector<vector<int>>> const &f2,
                int *const indx, const int &initL, const int &initR, paramT *const &P, const vector<int> &NucConst,
                const vector<vector<int>> &pos2nuc, const int &NCflg, int *const &i2r, int const &length, int const &w,
                int const (&BP_pair)[5][5], char *const &i2n, int *const &rtype, int *const &ii2r,
                vector<vector<int>> &Dep1, vector<vector<int>> &Dep2, int &DEPflg,
                vector<vector<vector<vector<pair<int, string>>>>> &, map<string, int> &predefE,
                vector<vector<vector<string>>> &substr, map<char, int> &n2i, const char *nucdef);

/*
string init_string(int const &len){
        string s;
        s.resize(len+1);
        s[0] = ' ';
        for(int i = 1; i <= len; i++){
                s[i] = 'N';
        }
        return s;
}
*/

inline auto TermAU(int const &type, paramT *const &P) -> int {
    if (type > 2) {
        return P->TerminalAU;
    }
    return 0;
}

inline auto E_hairpin(int size, int type, int si1, int sj1, const char *string, paramT *P) -> int {
    // AMW: change to strcpy from strncpy here improves safety
    // but may result in a 0.05% decrease in performance. May
    // be within error.
    int energy = (size <= 30) ? P->hairpin[size] : P->hairpin[30] + (int)(P->lxc * log((size) / 30.));
    // fprintf(stderr, "ok\n");
    if (P->model_details.special_hp) {
        char tl[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, *ts;
        if (size == 4) { /* check for tetraloop bonus */
            strcpy(tl, string);
            if ((ts = strstr(P->Tetraloops, tl))) {
                return (P->Tetraloop_E[(ts - P->Tetraloops) / 7]);
            }
        } else if (size == 6) {
            strcpy(tl, string);
            if ((ts = strstr(P->Hexaloops, tl))) {
                return (energy = P->Hexaloop_E[(ts - P->Hexaloops) / 9]);
            }
        } else if (size == 3) {
            strcpy(tl, string);
            if ((ts = strstr(P->Triloops, tl))) {
                return (P->Triloop_E[(ts - P->Triloops) / 6]);
            }
            return (energy + (type > 2 ? P->TerminalAU : 0));
        }
    }
    energy += P->mismatchH[type][si1][sj1];

    return energy;
}

inline auto E_intloop(int n1, int n2, int type, int type_2, int si1, int sj1, int sp1, int sq1, paramT *P) -> int {
    /* compute energy of degree 2 loop (stack bulge or interior) */
    int nl, ns, energy;
    energy = INF;
    int MAX_NINIO = 300;
    if (n1 > n2) {
        nl = n1;
        ns = n2;
    } else {
        nl = n2;
        ns = n1;
    }

    if (nl == 0)
        return P->stack[type][type_2]; /* stack */

    if (ns == 0) { /* bulge */
        energy = (nl <= MAXLOOP) ? P->bulge[nl] : (P->bulge[30] + (int)(P->lxc * log(nl / 30.)));
        if (nl == 1)
            energy += P->stack[type][type_2];
        else {
            if (type > 2)
                energy += P->TerminalAU;
            if (type_2 > 2)
                energy += P->TerminalAU;
        }
        return energy;
    } else { /* interior loop */
        if (ns == 1) {
            if (nl == 1) /* 1x1 loop */
                return P->int11[type][type_2][si1][sj1];
            if (nl == 2) { /* 2x1 loop */
                if (n1 == 1)
                    energy = P->int21[type][type_2][si1][sq1][sj1];
                else
                    energy = P->int21[type_2][type][sq1][si1][sp1];
                return energy;
            } else { /* 1xn loop */
                energy = (nl + 1 <= MAXLOOP) ? (P->internal_loop[nl + 1])
                                             : (P->internal_loop[30] + (int)(P->lxc * log((nl + 1) / 30.)));
                energy += MIN2(MAX_NINIO, (nl - ns) * P->ninio[2]);
                energy += P->mismatch1nI[type][si1][sj1] + P->mismatch1nI[type_2][sq1][sp1];
                return energy;
            }
        } else if (ns == 2) {
            if (nl == 2) { /* 2x2 loop */
                return P->int22[type][type_2][si1][sp1][sq1][sj1];
            } else if (nl == 3) { /* 2x3 loop */
                energy = P->internal_loop[5] + P->ninio[2];
                energy += P->mismatch23I[type][si1][sj1] + P->mismatch23I[type_2][sq1][sp1];
                return energy;
            }
        }
        { /* generic interior loop (no else here!)*/
            energy = (n1 + n2 <= MAXLOOP) ? (P->internal_loop[n1 + n2])
                                          : (P->internal_loop[30] + (int)(P->lxc * log((n1 + n2) / 30.)));
            energy += MIN2(MAX_NINIO, (nl - ns) * P->ninio[2]);

            energy += P->mismatchI[type][si1][sj1] + P->mismatchI[type_2][sq1][sp1];
        }
    }
    return energy;
}

auto getMemoryUsage(const string &fname) -> int {
    // cout << fname << endl;
    ifstream ifs(fname.c_str());
    if (ifs) {
        string line;
        while (getline(ifs, line)) { // $B9T$NFI$_9~$_(B
            string index = line.substr(0, 6);
            if (index == "VmRSS:") {
                // cout << line << endl;
                string mem_str;
                for (unsigned int i = 6; i < line.size(); i++) {
                    if (line[i] == ' ' || line[i] == '\t')
                        continue;
                    if (isdigit(line[i])) {
                        mem_str.append(1, line[i]);
                    } else if (line[i] == 'k' || line[i] == 'B') {
                        continue;
                    } else {
                        cerr << "Unexpected letter found in " << fname << "(" << line[i] << ")" << endl;
                        return -1;
                    }
                }
                stringstream ss(mem_str);
                int val;
                ss >> val;
                return val;
            }
        }

    } else {
        cerr << "Error: cannot open file(" << fname << ")" << endl;
        return -1;
    }

    cerr << "VmRSS line was not found in " << fname << endl;
    return -1;
}

void fill_optseq(string *optseq, int I, int J, vector<vector<int>> &pos2nuc, vector<vector<int>> Dep1) {

    int i2r[20], ii2r[100];
    map<char, int> n2i = make_n2i();
    char i2n[20];
    make_i2n(i2n);
    make_i2r(i2r);
    make_ii2r(ii2r); // encoding a dinucleotide (each eight variation) to an integer from 11 - 88

    // main routine
    //	cout << I << endl;
    //	cout << pos2nuc[I][0]<< endl;
    //	cout << i2n[pos2nuc[I][0]] << endl;
    (*optseq)[I] = i2n[pos2nuc[I][0]];
    //	cout << "CHK:" << (*optseq)[I] << endl;
    for (int i = I + 1; i <= J; i++) {
        for (unsigned int L = 0; L < pos2nuc[i].size(); L++) { // search possible nucleotides
            int L_nuc = pos2nuc[i][L];
            int L1_nuc;
            L1_nuc = n2i[(*optseq)[i - 1]];
            if (Dep1[ii2r[L1_nuc * 10 + L_nuc]][i - 1] == 0) {
                continue;
            }
            (*optseq)[i] = i2n[L_nuc];
            break;
        }
    }

    // modification of bases V,W,X,Y
    for (int i = I; i <= J; i++) {
        if ((*optseq)[i] == 'V' || (*optseq)[i] == 'W') {
            (*optseq)[i] = 'U';
        } else if ((*optseq)[i] == 'X' || (*optseq)[i] == 'Y') {
            (*optseq)[i] = 'G';
        }
    }
}

void fixed_init_matrix(const int &nuclen, const int &size, vector<int> & C, vector<int> & M, int *F, int *DMl, int *DMl1, int *DMl2) {
    for (int i = 0; i <= nuclen; i++) {
        F[i] = 0;
        DMl[i] = INF;
        DMl1[i] = INF;
        DMl2[i] = INF;
    }

    for (int i = 0; i < size; i++) {
        C[i] = INF;
        M[i] = INF;
    }
}

void fixed_backtrack(string optseq, bond *base_pair, vector<int> const & c, vector<int> const & m, int *f, int *indx, paramT *P, int nuclen, int w,
                     const int (&BP_pair)[5][5], map<string, int> predefE);

void fixed_fold(string optseq, int *indx, const int &w, map<string, int> &predefE, const int (&BP_pair)[5][5],
                paramT *P, char *aaseq, const codon& codon_table);