//
// Created by lenovo on 2021/11/2.
//

#include <stdio.h>
#include <string.h>
#include "tpmms.h"
#include "rw_tool.h"
// 一个子集合有 6 个块（不能平均）
const int group_size = 6;
unsigned char *r_blk[8], * w_blk;

void first_sort(Buffer *buf, int disk_start, int disk_end, int out_disk_num){
    int now = disk_start, group_num = (disk_end - disk_start + group_size) / group_size;
    // 一共应该分为 (disk_end - disk_start + 1 + group_size - 1) / group_size 组，即向上取整
    for(int group = 0;group < group_num;group ++){
        int cnt = 0;
        while(cnt < group_size && now <= disk_end){
            if ((r_blk[cnt] = readBlockFromDisk(now, buf)) == NULL){
                perror("Reading Block Failed!\n");
                return;
            }
            printf("读入数据块%d\n", now);
            now = getAddr(r_blk[cnt]);
            cnt++;
        }
        // 采用不优化的冒泡排序，外层循环要循环多少次，即一个 group 中所有块 * 元祖数
        int outer_cnt = cnt * 8;
        for(int i = 0;i < outer_cnt;i++){
            for(int j = 0; j < cnt ;j++){
                // 注意这里是 56，因为后 8 位是地址
                for(int b = 0;b < 56;b += 8){
                    unsigned char * bx,*by;
                    bx = r_blk[j] + b;
                    // 当已经是这个缓存块中最后一个元组时，要和下一个缓存块的第一个元组比较
                    if(b == 48){
                        if(j == cnt - 1) continue;
                        by = r_blk[j + 1];
                    }else
                        by = r_blk[j] + b + 8;
                    if(cmp(bx, by))
                        blk_swap(bx, by);
                }
            }
        }
        for(int i = 0; i < cnt ; i++){
            writeXY_blk(r_blk[i] + 56, out_disk_num + i + 1, NEXT_FLAG);
            if (writeBlockToDisk(r_blk[i], out_disk_num + i, buf) != 0){
                perror("Writing Block Failed!\n");
                return;
            }
            printf("注：结果写入磁盘：%d\n",out_disk_num + i);
            // 不需要 freeBlockInBuffer(r_blk[i], buf); writeBlockToDisk 会自动释放
        }
        out_disk_num += group_size;
    }
}

void second_sort(Buffer *buf, int disk_start, int disk_end, int out_disk_num){
    int group_num = (disk_end - disk_start + group_size) / group_size;
    // disk_index 记录每个缓存块中的磁盘块号
    int disk_index[8];
    for(int group = 0;group < group_num ;group++ ){
        disk_index[group] = disk_start + group_size * group;
        if ((r_blk[group] = readBlockFromDisk(disk_index[group], buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("读入数据块%d\n", disk_index[group]);
    }
    // disk_cnt 记录已处理完几个磁盘块，index 记录每个缓存块当前比较到的位置
    // cnt 写出缓存块的记录数
    int disk_cnt = 0, index[8] = {0}, cnt = 0;
    while(disk_cnt < disk_end - disk_start + 1){
        // 最小值所在组和偏移
        int mi_g = -1, mi_index = 0;
        for(int group = 0;group < group_num;group++){
            // 判断这个缓存块是否为空（即这个组已经空了）
            if(disk_index[group] > disk_end || disk_index[group] >= disk_start + group_size * (group + 1))
                continue;
            if(mi_g == -1 || cmp(r_blk[mi_g] + mi_index,r_blk[group] + index[group] )){
                mi_g = group;
                mi_index = index[group];
            }
        }
        // 如果写出块满了，要先写出到磁盘
        if(cnt && cnt % 7 == 0){
            write_new_disk(w_blk, buf, out_disk_num, PRINT);
            out_disk_num ++;
        }

        int x = getX(r_blk[mi_g] + mi_index);
        int y = getY(r_blk[mi_g] + mi_index);
        writeXY_blk(w_blk + 8 * (cnt % 7), x, y);
        index[mi_g] += 8;
        cnt++;
        // 要从这个组中读入一个新块
        if(mi_index == 48){
            freeBlockInBuffer(r_blk[mi_g], buf);
            disk_index[mi_g]++;
            disk_cnt++;
            // 如果这个组已经没有新块就跳过
            if(disk_index[mi_g] > disk_end || disk_index[mi_g] >= disk_start + group_size * (mi_g + 1))
                continue;
            if ((r_blk[mi_g] = readBlockFromDisk(disk_index[mi_g], buf)) == NULL){
                perror("Reading Block Failed!\n");
                return;
            }
            printf("读入数据块%d\n", disk_index[mi_g]);
            index[mi_g] = 0;
        }
    }
    // 当前写出缓存块还有值，得写入磁盘块（写入一个越界的地址）
    writeXY_blk(w_blk + 56, out_disk_num + 1, NEXT_FLAG);
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    *(w_blk - 1) = BLOCK_UNAVAILABLE;
    printf("注：结果写入磁盘：%d\n",out_disk_num);
}

void tpmms(Buffer *buf){
    printf("\n--------------------------------\n");
    printf(RED"两阶段多路归并排序算法\n"WHITE);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }

    // w_blk 是用于把结果写出到磁盘的缓存块，可以存 7 个元组，最后放下一个缓存块地址
    w_blk = getNewBlockInBuffer(buf);
    // 缓存块不一定没有值，要清零
    memset(w_blk, 0, 64);
    // 对 R 关系排序

    printf("\n--------------------------------\n");
    printf("R 的第一趟排序\n");
    printf("--------------------------------\n");
    first_sort(buf, 1, 16, TPMMS_FIRST_DISK_NUM);

    printf("\n--------------------------------\n");
    printf("R 的第二趟排序\n");
    printf("--------------------------------\n");
    second_sort(buf, TPMMS_FIRST_DISK_NUM, TPMMS_FIRST_DISK_NUM + 15, TPMMS_SECOND_DISK_NUM);

    // 对 S 关系排序
    printf("\n--------------------------------\n");
    printf("S 的第一趟排序\n");
    printf("--------------------------------\n");
    first_sort(buf, 17, 48, TPMMS_FIRST_DISK_NUM + 16);

    printf("\n--------------------------------\n");
    printf("S 的第二趟排序\n");
    printf("--------------------------------\n");
    second_sort(buf, TPMMS_FIRST_DISK_NUM + 16, TPMMS_FIRST_DISK_NUM + 47, TPMMS_SECOND_DISK_NUM + 16);

    printf(RED"\nIO读写一共%lu次."WHITE, buf->numIO);
    freeBuffer(buf);
}
