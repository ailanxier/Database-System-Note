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
    printf(RED"��������������ѡ���㷨 S.C = %d\n"WHITE, target_num);
    printf("--------------------------------\n");

    unsigned char * r_blk, * w_blk;
    // w_blk �����ڰѽ��д�������̵Ļ���飬���Դ� 7 ��Ԫ�飬������һ��������ַ
    w_blk = getNewBlockInBuffer(buf);
    // cnt ��¼���ҵ��ļ�¼��,now Ϊ��ǰ�Ĵ��̿�ţ���ַ��
    int cnt = 0,now = 17;
    // ����鲻һ��û��ֵ��Ҫ����
    memset(w_blk, 0, 64);

    while(now <= 48){
        // r_blk �����ڴӴ��̶�ȡ���ݵĻ����
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("�������ݿ�%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            int y = getY(r_blk + i * 8);
            // SC = 130
            if(x == target_num){
                printf("(X = %d,Y = %d)\n",x,y);
                // д������Ѿ��� 7 ��Ԫ���ˣ�����Ҫ������Ԫ�飬���Ȱ�ԭ��������д�ش���
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

    // ��ǰд������黹��ֵ����д����̿�
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("ע�����д����̣�%d\n",out_disk_num);

    printf(RED"\n����ѡ��������Ԫ��һ��%d��.\n\nIO��дһ��%lu��."WHITE, cnt, buf->numIO);
    freeBuffer(buf);
}
