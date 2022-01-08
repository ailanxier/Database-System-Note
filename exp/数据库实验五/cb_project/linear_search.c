//
// Created by lenovo on 2021/11/2.
//

#include "linear_search.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void linear_search(Buffer *buf, int target_num){
    int out_disk_num = LINEAR_SEARCH_DISK_NUM;
    printf("--------------------------------\n");
    printf(RED"基于线性搜索的选择算法 S.C = %d\n"WHITE, target_num);
    printf("--------------------------------\n");

    unsigned char * r_blk, * w_blk;
    // w_blk 是用于把结果写出到磁盘的缓存块，可以存 7 个元组，最后放下一个缓存块地址
    w_blk = getNewBlockInBuffer(buf);
    // cnt 记录查找到的记录数,now 为当前的磁盘块号（地址）
    int cnt = 0,now = 17;
    // 缓存块不一定没有值，要清零
    memset(w_blk, 0, 64);

    while(now <= 48){
        // r_blk 是用于从磁盘读取数据的缓存块
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("读入数据块%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            int y = getY(r_blk + i * 8);
            // SC = 130
            if(x == target_num){
                printf("(X = %d,Y = %d)\n",x,y);
                // 写缓存块已经满 7 个元组了，现在要插入新元组，得先把原来的数据写回磁盘
                if(cnt && cnt % 7 == 0){
                    write_new_disk(w_blk, buf, out_disk_num, PRINT);
                    out_disk_num ++;
                }
                writeXY_blk(w_blk + 8 * (cnt % 7), x, y);
                cnt++;
            }
        }
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
    }

    // 当前写出缓存块还有值，得写入磁盘块
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("注：结果写入磁盘：%d\n",out_disk_num);

    printf(RED"\n满足选择条件的元组一共%d个.\n\nIO读写一共%lu次."WHITE, cnt, buf->numIO);
    freeBuffer(buf);
}
