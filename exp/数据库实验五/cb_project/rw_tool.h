//
// Created by lenovo on 2021/11/2.
//

#include "extmem.h"

#ifndef EXTMEM_C_RW_TOOL_H
#define EXTMEM_C_RW_TOOL_H
#define NONE         "\033[m"
#define WHITE        "\033[1;37m"
#define RED          "\033[0;32;31m"

#define NEXT_FLAG                       0
#define PRINT                           1
#define NOT_PRINT                       0
#define LINEAR_SEARCH_DISK_NUM         100
#define TPMMS_FIRST_DISK_NUM           201
#define TPMMS_SECOND_DISK_NUM          301
#define INDEX_DISK_NUM                 350
#define INDEX_SEARCH_DISK_NUM          120
#define SORT_MERGE_JOIN_DISK_NUM       401
#define UNION_DISK_NUM                 801
#define INTERSECT_DISK_NUM             140
#define SUBTRACT_DISK_NUM              901


int getX(unsigned char *);
int getY(unsigned char *);
int getAddr(unsigned char *);
void writeXY_blk(unsigned char *, int, int);
void write_new_disk(unsigned char *, Buffer *, int, int);
int XY_less(unsigned char *, unsigned char *);
int X_less(unsigned char *, unsigned char *);
int X_equal(unsigned char *, unsigned char *);
int XY_equal(unsigned char *,unsigned char *);
int xy_equal(unsigned char *, int, int);
void blk_swap(unsigned char *, unsigned char *);
int cmp(unsigned char *,unsigned char *);

#endif //EXTMEM_C_RW_TOOL_H
