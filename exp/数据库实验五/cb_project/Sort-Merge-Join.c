//
// Created by lenovo on 2021/11/3.
//

#include "Sort-Merge-Join.h"
#include <stdio.h>
#include <string.h>
#include "rw_tool.h"

// ֱ������ tpmms �Ѿ��ź�������ݿ��������
void sort_merge_join(Buffer * buf){
    printf("\n--------------------------------\n");
    printf(RED"��������������㷨\n"WHITE);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }
    int out_disk_num = SORT_MERGE_JOIN_DISK_NUM;
    // �����ϵ��Ԫ����
    int cnt = 0;
    // r_blk ���� R ��ϵ�����ݿ飬s_blk ���� S ��ϵ�����ݿ�
    unsigned char * r_blk, * s_blk ,* w_blk, *start_blk;
    w_blk = getNewBlockInBuffer(buf);
    start_blk = getNewBlockInBuffer(buf);
    memset(w_blk, 0, 64);
    // r_now Ϊ��ǰ r_blk �Ĵ��̿�ţ�s_now Ϊ��ǰ s_blk �Ĵ��̿��
    int r_now = TPMMS_SECOND_DISK_NUM, s_now = r_now + 16;
    int r_end = r_now + 16, s_end = s_now + 32, r_index = 0, s_index = 0, start_disk, start_index;
    if ((r_blk = readBlockFromDisk(r_now, buf)) == NULL){
        perror("Reading Block Failed!\n");
        return;
    }
    if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
        perror("Reading Block Failed!\n");
        return;
    }
    strcpy(start_blk, s_blk);
    while(r_now < r_end && s_now < s_end){
        start_disk = s_now;
        start_index = s_index;
        while(X_equal(r_blk + r_index, s_blk + s_index)){
            // һ�����Ӽ�¼ռ����Ԫ��
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), getX(r_blk + r_index), getY(r_blk + r_index));
            cnt++;
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), getX(s_blk + s_index), getY(s_blk + s_index));
            cnt++;
            // s ����
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
        if(s_now != start_index){
            s_now = start_disk;
            strcpy(s_blk, start_blk);
        }
        s_index = start_index;
        // r ����
        if(X_less(r_blk + r_index, s_blk + s_index)){
            if(r_index == 48){
                r_now = getAddr(r_blk);
                freeBlockInBuffer(r_blk, buf);
                if(r_now >= r_end) break;
                if ((r_blk = readBlockFromDisk(r_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                r_index = 0;
            }else
                r_index += 8;
        }else{
            // s ����
            if(s_index == 48){
                s_now = getAddr(s_blk);
                freeBlockInBuffer(s_blk, buf);
                if(s_now >= s_end) break;
                if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
                strcpy(start_blk, s_blk);
                s_index = 0;
            }else
                s_index += 8;
        }
    }
    // ��ǰд������黹��ֵ����д����̿�
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("ע�����д����̣�%d\n",out_disk_num);

    printf(RED"\n����ѡ��������Ԫ��һ��%d��.\n\nIO��дһ��%lu��."WHITE, cnt/2, buf->numIO);

    freeBuffer(buf);
}
