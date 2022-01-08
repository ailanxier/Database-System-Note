//
// Created by lenovo on 2021/11/2.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rw_tool.h"
char s[8];

int getX(unsigned char *blk){
    // x Ϊ 3 λ��
    for (int i = 0; i < 3; i++)
        s[i] = blk[i];
    s[3] = '\0';
    return atoi(s);
}

int getY(unsigned char *blk){
    // y Ϊ 4 λ��
    for (int i = 0; i < 4; i++)
        s[i] = blk[i + 4];
    s[4] = '\0';
    return atoi(s);
}

int getAddr(unsigned char *blk){
    // y Ϊ 4 λ��
    for (int i = 0; i < 4; i++)
        s[i] = blk[i + 56];
    s[4] = '\0';
    return atoi(s);
}

void writeXY_blk(unsigned char *blk, int x, int y){
    // x Ϊ 3 λ��
    for(int i = 0;i < 3;i++){
        blk[2 - i] = x % 10 + '0';
        x /= 10;
    }
    blk[3] = ' ';
    // ������ y = next_flag ʱ�ǿ������ĸ��ֽڣ�ָ����һ��ַ�����Ժ������ַ�
    if(y == NEXT_FLAG) blk[3] = '\0';
    else{
        for(int i = 0;i < 4;i++){
            blk[7 - i] = y % 10 + '0';
            y /= 10;
        }
    }
}

void write_new_disk(unsigned char * w_blk, Buffer * buf, int out_disk_num, int print_flag){
    // д����һ���ַ
    writeXY_blk(w_blk + 56, out_disk_num + 1, NEXT_FLAG);
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    if(print_flag == PRINT)
        printf("ע�����д����̣�%d\n",out_disk_num);
    memset(w_blk, 0, 64);
    // ��ʹ�� writeBlockToDisk ��ѻ��������Ϊ���ã�Ҫ�ֶ�����Ϊ�����ã���Ϊ�Ҳ����˸���ͬһ�������ķ�ʽ��
    *(w_blk - 1) = BLOCK_UNAVAILABLE;
}

int XY_less(unsigned char *a, unsigned char *b){
    int ax = getX(a),ay = getY(a);
    int bx = getX(b),by = getY(b);
    return ax == bx ? ay < by : ax < bx;
}

int X_less(unsigned char *a, unsigned char *b){
    int ax = getX(a);
    int bx = getX(b);
    return ax <= bx;
}

int X_equal(unsigned char *a, unsigned char *b){
    int ax = getX(a);
    int bx = getX(b);
    return ax == bx;
}

int XY_equal(unsigned char *a,unsigned char *b){
    int ax = getX(a),ay = getY(a);
    int bx = getX(b),by = getY(b);
    return ax == bx && ay == by;
}

int xy_equal(unsigned char *a, int x,int y){
    int ax = getX(a),ay = getY(a);
    return ax == x && ay == y;
}

void blk_swap(unsigned char *a, unsigned char *b){
    int ax = getX(a), ay = getY(a);
    int bx = getX(b), by = getY(b);
    writeXY_blk(a, bx, by);
    writeXY_blk(b, ax, ay);
}

// Ԫ��ȽϺ�����x Ϊ��һ�ؼ��֣�y Ϊ�ڶ��ؼ��֣��� a ���򷵻� 1��
int cmp(unsigned char *a,unsigned char *b){
    int ax = getX(a), ay = getY(a);
    int bx = getX(b), by = getY(b);
    return ax == bx ? ay > by : ax > bx;
}
