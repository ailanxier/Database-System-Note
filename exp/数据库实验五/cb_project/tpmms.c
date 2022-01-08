//
// Created by lenovo on 2021/11/2.
//

#include <stdio.h>
#include <string.h>
#include "tpmms.h"
#include "rw_tool.h"
// һ���Ӽ����� 6 ���飨����ƽ����
const int group_size = 6;
unsigned char *r_blk[8], * w_blk;

void first_sort(Buffer *buf, int disk_start, int disk_end, int out_disk_num){
    int now = disk_start, group_num = (disk_end - disk_start + group_size) / group_size;
    // һ��Ӧ�÷�Ϊ (disk_end - disk_start + 1 + group_size - 1) / group_size �飬������ȡ��
    for(int group = 0;group < group_num;group ++){
        int cnt = 0;
        while(cnt < group_size && now <= disk_end){
            if ((r_blk[cnt] = readBlockFromDisk(now, buf)) == NULL){
                perror("Reading Block Failed!\n");
                return;
            }
            printf("�������ݿ�%d\n", now);
            now = getAddr(r_blk[cnt]);
            cnt++;
        }
        // ���ò��Ż���ð���������ѭ��Ҫѭ�����ٴΣ���һ�� group �����п� * Ԫ����
        int outer_cnt = cnt * 8;
        for(int i = 0;i < outer_cnt;i++){
            for(int j = 0; j < cnt ;j++){
                // ע�������� 56����Ϊ�� 8 λ�ǵ�ַ
                for(int b = 0;b < 56;b += 8){
                    unsigned char * bx,*by;
                    bx = r_blk[j] + b;
                    // ���Ѿ����������������һ��Ԫ��ʱ��Ҫ����һ�������ĵ�һ��Ԫ��Ƚ�
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
            printf("ע�����д����̣�%d\n",out_disk_num + i);
            // ����Ҫ freeBlockInBuffer(r_blk[i], buf); writeBlockToDisk ���Զ��ͷ�
        }
        out_disk_num += group_size;
    }
}

void second_sort(Buffer *buf, int disk_start, int disk_end, int out_disk_num){
    int group_num = (disk_end - disk_start + group_size) / group_size;
    // disk_index ��¼ÿ��������еĴ��̿��
    int disk_index[8];
    for(int group = 0;group < group_num ;group++ ){
        disk_index[group] = disk_start + group_size * group;
        if ((r_blk[group] = readBlockFromDisk(disk_index[group], buf)) == NULL){
            perror("Reading Block Failed!\n");
            return;
        }
        printf("�������ݿ�%d\n", disk_index[group]);
    }
    // disk_cnt ��¼�Ѵ����꼸�����̿飬index ��¼ÿ������鵱ǰ�Ƚϵ���λ��
    // cnt д�������ļ�¼��
    int disk_cnt = 0, index[8] = {0}, cnt = 0;
    while(disk_cnt < disk_end - disk_start + 1){
        // ��Сֵ�������ƫ��
        int mi_g = -1, mi_index = 0;
        for(int group = 0;group < group_num;group++){
            // �ж����������Ƿ�Ϊ�գ���������Ѿ����ˣ�
            if(disk_index[group] > disk_end || disk_index[group] >= disk_start + group_size * (group + 1))
                continue;
            if(mi_g == -1 || cmp(r_blk[mi_g] + mi_index,r_blk[group] + index[group] )){
                mi_g = group;
                mi_index = index[group];
            }
        }
        // ���д�������ˣ�Ҫ��д��������
        if(cnt && cnt % 7 == 0){
            write_new_disk(w_blk, buf, out_disk_num, PRINT);
            out_disk_num ++;
        }

        int x = getX(r_blk[mi_g] + mi_index);
        int y = getY(r_blk[mi_g] + mi_index);
        writeXY_blk(w_blk + 8 * (cnt % 7), x, y);
        index[mi_g] += 8;
        cnt++;
        // Ҫ��������ж���һ���¿�
        if(mi_index == 48){
            freeBlockInBuffer(r_blk[mi_g], buf);
            disk_index[mi_g]++;
            disk_cnt++;
            // ���������Ѿ�û���¿������
            if(disk_index[mi_g] > disk_end || disk_index[mi_g] >= disk_start + group_size * (mi_g + 1))
                continue;
            if ((r_blk[mi_g] = readBlockFromDisk(disk_index[mi_g], buf)) == NULL){
                perror("Reading Block Failed!\n");
                return;
            }
            printf("�������ݿ�%d\n", disk_index[mi_g]);
            index[mi_g] = 0;
        }
    }
    // ��ǰд������黹��ֵ����д����̿飨д��һ��Խ��ĵ�ַ��
    writeXY_blk(w_blk + 56, out_disk_num + 1, NEXT_FLAG);
    if (writeBlockToDisk(w_blk, out_disk_num, buf) != 0){
        perror("Writing Block Failed!\n");
        return;
    }
    *(w_blk - 1) = BLOCK_UNAVAILABLE;
    printf("ע�����д����̣�%d\n",out_disk_num);
}

void tpmms(Buffer *buf){
    printf("\n--------------------------------\n");
    printf(RED"���׶ζ�·�鲢�����㷨\n"WHITE);
    printf("--------------------------------\n");
    if (!initBuffer(520, 64, buf)){
        perror("Buffer Initialization Failed!\n");
        return;
    }

    // w_blk �����ڰѽ��д�������̵Ļ���飬���Դ� 7 ��Ԫ�飬������һ��������ַ
    w_blk = getNewBlockInBuffer(buf);
    // ����鲻һ��û��ֵ��Ҫ����
    memset(w_blk, 0, 64);
    // �� R ��ϵ����

    printf("\n--------------------------------\n");
    printf("R �ĵ�һ������\n");
    printf("--------------------------------\n");
    first_sort(buf, 1, 16, TPMMS_FIRST_DISK_NUM);

    printf("\n--------------------------------\n");
    printf("R �ĵڶ�������\n");
    printf("--------------------------------\n");
    second_sort(buf, TPMMS_FIRST_DISK_NUM, TPMMS_FIRST_DISK_NUM + 15, TPMMS_SECOND_DISK_NUM);

    // �� S ��ϵ����
    printf("\n--------------------------------\n");
    printf("S �ĵ�һ������\n");
    printf("--------------------------------\n");
    first_sort(buf, 17, 48, TPMMS_FIRST_DISK_NUM + 16);

    printf("\n--------------------------------\n");
    printf("S �ĵڶ�������\n");
    printf("--------------------------------\n");
    second_sort(buf, TPMMS_FIRST_DISK_NUM + 16, TPMMS_FIRST_DISK_NUM + 47, TPMMS_SECOND_DISK_NUM + 16);

    printf(RED"\nIO��дһ��%lu��."WHITE, buf->numIO);
    freeBuffer(buf);
}
