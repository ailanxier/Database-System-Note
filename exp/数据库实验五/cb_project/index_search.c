//
// Created by lenovo on 2021/11/3.
//

#include <stdio.h>
#include <string.h>
#include "tpmms.h"
#include "rw_tool.h"
// 满足关系的元组数
int cnt = 0;

void build_index(Buffer *buf, int out_disk_num){
    unsigned char * r_blk, * w_blk;
    // w_blk 是用于把结果写出到磁盘的缓存块，可以存 7 个元组，最后放下一个缓存块地址
    w_blk = getNewBlockInBuffer(buf);
    // now 为当前磁盘块号
    int now = TPMMS_SECOND_DISK_NUM + 16;
    int end = TPMMS_SECOND_DISK_NUM + 48;
    // 缓存块不一定没有值，要清零
    memset(w_blk, 0, 64);

    // 当前要给 num = S.C 建立索引，按升序第一个出现 num 的磁盘块号为 now
    int num = 0;
    while(now < end){
        // r_blk 是用于从磁盘读取数据的缓存块
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        //printf("读入数据库%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            if(x != num){
                // 写缓存块已经满 7 个元组了，现在要插入新元组，得先把原来的数据写回磁盘
                if(cnt && cnt % 7 == 0){
                    write_new_disk(w_blk, buf, out_disk_num, NOT_PRINT);
                    out_disk_num ++;
                }
                // X 是索引字段，now 是磁盘块号，乘 10 是因为原本 Y 有 4 位
                writeXY_blk(w_blk + 8 * (cnt % 7), x, now * 10);
                cnt++;
            }
            num = x;
        }
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
    }

    // 当前写出缓存块还有值，得写入磁盘块
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
}

void find_by_index(Buffer *buf, int target_num, int out_disk_num){
    unsigned char * r_blk, * w_blk;
    w_blk = getNewBlockInBuffer(buf);
    // cnt 记录查找到的记录数,now 为当前的缓存块号（地址）
    int now = INDEX_DISK_NUM;
    // 缓存块不一定没有值，要清零
    memset(w_blk, 0, 64);

    // 索引到的磁盘块号
    int disk_num = 0;
    while(1){
        // r_blk 是用于从磁盘读取数据的缓存块
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("读入索引块%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            int y = getY(r_blk + i * 8);
            if(x > target_num)
                break;
            // S.C = 130
            if(x == target_num){
                disk_num = y / 10;
                // 如果当前是最后一个元组，要去下一块找元组
                break;
            }
        }
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
        // 如果已经找到或者已经无后续索引块了，就退出
        if(disk_num || now == 0)
            break;
        printf("没有满足条件的元组。\n");
    }
    if(!disk_num){
        printf("没有满足 S.C = %d 的元组。\n",target_num);
        return;
    }
    // 根据索引找数据块
    now = disk_num;
    int first_meet = 0, not_meet = 0;
    cnt = 0;
    while(1){
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("读入数据库块%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            int y = getY(r_blk + i * 8);
            if(x != target_num){
                if(!first_meet) continue;
                not_meet = 1;
                break;
            }
            if(x == target_num){
                first_meet = 1;
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
        if(not_meet) break;
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
    }

    // 当前写出缓存块还有值，得写入磁盘块
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("注：结果写入磁盘：%d\n",out_disk_num);
}

void index_search(Buffer *buf, int target_num){
    printf("\n--------------------------------\n");
    printf(RED"基于索引的选择算法 S.C = %d\n"WHITE, target_num);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }
    build_index(buf, INDEX_DISK_NUM);
    freeBuffer(buf);
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }
    find_by_index(buf, target_num, INDEX_SEARCH_DISK_NUM);
    printf(RED"\n满足选择条件的元组一共%d个.\n\nIO读写一共%lu次."WHITE, cnt, buf->numIO);
    freeBuffer(buf);
}
