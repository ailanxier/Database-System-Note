//
// Created by lenovo on 2021/11/3.
//

#include <stdio.h>
#include <string.h>
#include "tpmms.h"
#include "rw_tool.h"
// �����ϵ��Ԫ����
int cnt = 0;

void build_index(Buffer *buf, int out_disk_num){
    unsigned char * r_blk, * w_blk;
    // w_blk �����ڰѽ��д�������̵Ļ���飬���Դ� 7 ��Ԫ�飬������һ��������ַ
    w_blk = getNewBlockInBuffer(buf);
    // now Ϊ��ǰ���̿��
    int now = TPMMS_SECOND_DISK_NUM + 16;
    int end = TPMMS_SECOND_DISK_NUM + 48;
    // ����鲻һ��û��ֵ��Ҫ����
    memset(w_blk, 0, 64);

    // ��ǰҪ�� num = S.C �����������������һ������ num �Ĵ��̿��Ϊ now
    int num = 0;
    while(now < end){
        // r_blk �����ڴӴ��̶�ȡ���ݵĻ����
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        //printf("�������ݿ�%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            if(x != num){
                // д������Ѿ��� 7 ��Ԫ���ˣ�����Ҫ������Ԫ�飬���Ȱ�ԭ��������д�ش���
                if(cnt && cnt % 7 == 0){
                    write_new_disk(w_blk, buf, out_disk_num, NOT_PRINT);
                    out_disk_num ++;
                }
                // X �������ֶΣ�now �Ǵ��̿�ţ��� 10 ����Ϊԭ�� Y �� 4 λ
                writeXY_blk(w_blk + 8 * (cnt % 7), x, now * 10);
                cnt++;
            }
            num = x;
        }
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
    }

    // ��ǰд������黹��ֵ����д����̿�
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
}

void find_by_index(Buffer *buf, int target_num, int out_disk_num){
    unsigned char * r_blk, * w_blk;
    w_blk = getNewBlockInBuffer(buf);
    // cnt ��¼���ҵ��ļ�¼��,now Ϊ��ǰ�Ļ����ţ���ַ��
    int now = INDEX_DISK_NUM;
    // ����鲻һ��û��ֵ��Ҫ����
    memset(w_blk, 0, 64);

    // �������Ĵ��̿��
    int disk_num = 0;
    while(1){
        // r_blk �����ڴӴ��̶�ȡ���ݵĻ����
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("����������%d\n", now);
        for(int i = 0;i < 7;i++){
            int x = getX(r_blk + i * 8);
            int y = getY(r_blk + i * 8);
            if(x > target_num)
                break;
            // S.C = 130
            if(x == target_num){
                disk_num = y / 10;
                // �����ǰ�����һ��Ԫ�飬Ҫȥ��һ����Ԫ��
                break;
            }
        }
        now = getAddr(r_blk);
        freeBlockInBuffer(r_blk, buf);
        // ����Ѿ��ҵ������Ѿ��޺����������ˣ����˳�
        if(disk_num || now == 0)
            break;
        printf("û������������Ԫ�顣\n");
    }
    if(!disk_num){
        printf("û������ S.C = %d ��Ԫ�顣\n",target_num);
        return;
    }
    // �������������ݿ�
    now = disk_num;
    int first_meet = 0, not_meet = 0;
    cnt = 0;
    while(1){
        if ((r_blk = readBlockFromDisk(now, buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("�������ݿ��%d\n", now);
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
                // д������Ѿ��� 7 ��Ԫ���ˣ�����Ҫ������Ԫ�飬���Ȱ�ԭ��������д�ش���
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

    // ��ǰд������黹��ֵ����д����̿�
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    printf("ע�����д����̣�%d\n",out_disk_num);
}

void index_search(Buffer *buf, int target_num){
    printf("\n--------------------------------\n");
    printf(RED"����������ѡ���㷨 S.C = %d\n"WHITE, target_num);
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
    printf(RED"\n����ѡ��������Ԫ��һ��%d��.\n\nIO��дһ��%lu��."WHITE, cnt, buf->numIO);
    freeBuffer(buf);
}
