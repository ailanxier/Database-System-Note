//
// Created by lenovo on 2021/11/3.
//

#include "union.h"
#include <stdio.h>
#include <string.h>
#include "rw_tool.h"

// ֱ������ tpmms �Ѿ��ź�������ݿ���в�����
void Union(Buffer * buf){
    printf("\n--------------------------------\n");
    printf(RED"��������ļ��ϵĲ��㷨\n"WHITE);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }
    int out_disk_num = UNION_DISK_NUM;
    // �����ϵ��Ԫ����
    int cnt = 0;
    // r_blk ���� R ��ϵ�����ݿ飬s_blk ���� S ��ϵ�����ݿ�
    unsigned char * r_blk, * s_blk ,* w_blk;
    w_blk = getNewBlockInBuffer(buf);
    memset(w_blk, 0, 64);
    // r_now Ϊ��ǰ r_blk �Ĵ��̿�ţ�s_now Ϊ��ǰ s_blk �Ĵ��̿��
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
    while(r_now < r_end || s_now < s_end){
        // r ���� s �������ֻ���һ��
        if(r_now < r_end && s_now < s_end && XY_equal(r_blk + r_index, s_blk + s_index)){
            int x = getX(r_blk + r_index);
            int y = getY(r_blk + r_index);
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), x, y);
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
            // r ����
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
        // s Ϊ�� ���� r С�� s �������r ���������
        else if(s_now >= s_end || (r_now < r_end && XY_less(r_blk + r_index, s_blk + s_index)) ){
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), getX(r_blk + r_index), getY(r_blk + r_index));
            cnt++;
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
            // s ����
            if(cnt && cnt % 7 == 0){
                write_new_disk(w_blk, buf, out_disk_num, PRINT);
                out_disk_num ++;
            }
            writeXY_blk(w_blk + 8 * (cnt % 7), getX(s_blk + s_index), getY(s_blk + s_index));
            cnt++;
            if(s_index == 48){
                s_now = getAddr(s_blk);
                freeBlockInBuffer(s_blk, buf);
                if(s_now >= s_end) continue;
                if ((s_blk = readBlockFromDisk(s_now, buf)) == NULL){
                    perror("Reading Block Failed!\n");
                    return;
                }
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

    printf(RED"\nS��R�Ĳ�����%d��Ԫ�顣"WHITE, cnt);
    freeBuffer(buf);
}
