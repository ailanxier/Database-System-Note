//
// Created by lenovo on 2021/11/3.
//

#include "subtract.h"
#include <stdio.h>
#include <string.h>
#include "rw_tool.h"

// 直接利用 tpmms 已经排好序的数据库进行差S-R运算
void subtract(Buffer * buf){
    printf("\n--------------------------------\n");
    printf(RED"基于排序的集合的差算法\n"WHITE);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }
    int out_disk_num = SUBTRACT_DISK_NUM;
    // 满足关系的元组数
    int cnt = 0;
    // r_blk 读入 R 关系的数据块，s_blk 读入 S 关系的数据块
    unsigned char * r_blk, * s_blk ,* w_blk;
    w_blk = getNewBlockInBuffer(buf);
    memset(w_blk, 0, 64);
    // r_now 为当前 r_blk 的磁盘块号，s_now 为当前 s_blk 的磁盘块号
    int r_now = TPMMS_SECOND_DISK_NUM, s_now = r_now + 16;
    int r_end = r_now + 16, s_end = s_now + 32, r_index = 0, s_index = 0;
    if ((r_blk = readBlockFromDisk(r_now, buf)) == NULL){
        perror("Reading Block Failed!\n");
        return;
    }
    if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
        perror("Reading Block Failed!\n");
        return;
    }
    while(s_now < s_end){
        // r 等于 s 的情况，跳过不输出
        if(r_now < r_end && s_now < s_end && XY_equal(r_blk + r_index, s_blk + s_index)){
            // s 后移
            if(s_index == 48){
                s_now = getAddr(s_blk);
                freeBlockInBuffer(s_blk, buf);
                if(s_now >= s_end) break;
                if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                s_index = 0;
            }else
                s_index += 8;
            // r 后移
            if(r_index == 48){
                r_now = getAddr(r_blk);
                freeBlockInBuffer(r_blk, buf);
                if(r_now >= r_end) break;
                if ((r_blk = readBlockFromDisk(r_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                r_index = 0;
            }
            else
                r_index += 8;
        }
        // s 为空 或者 r 小于 s 的情况，r 输出并后移
        else if(r_now < r_end && XY_less(r_blk + r_index, s_blk + s_index)){
            if(r_index == 48){
                r_now = getAddr(r_blk);
                freeBlockInBuffer(r_blk, buf);
                if(r_now >= r_end) continue;
                if ((r_blk = readBlockFromDisk(r_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                r_index = 0;
            }else
                r_index += 8;
        }else{
            // s 后移
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), getX(s_blk + s_index), getY(s_blk + s_index));
            cnt++;
            if(s_index == 48){
                s_now = getAddr(s_blk);
                freeBlockInBuffer(s_blk, buf);
                if(s_now >= s_end) break;
                if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                s_index = 0;
            }else
                s_index += 8;
        }
    }

    // 当前写出缓存块还有值，得写入磁盘块
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("注：结果写入磁盘：%d\n",out_disk_num);

    printf(RED"\nS和R的差集(S-R)有%d个元组。"WHITE, cnt);
    freeBuffer(buf);
}
