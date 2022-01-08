#include <stdio.h>
#include <stdlib.h>
#include "linear_search.h"
#include "tpmms.h"
#include "index_search.h"
#include "Sort-Merge-Join.h"
#include "intersect.h"
#include "union.h"
#include "subtract.h"

int main() {
    Buffer buf;
    if (!initBuffer(520, 64, &buf)){
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    system("color 07");

    // ʹ�����������㷨���ҳ�S.C = 130��Ԫ��
    // 100 ��ʼ�����Բ��ҽ��
    linear_search(&buf, 130);

    // ���׶ζ�·�鲢�����㷨
    // 201 - 248 �ŷ�����������ʱ�ļ�
    // ��ϵ R �����������ļ� 301.blk �� 316.blk
    // ��ϵ S �����������ļ� 317.blk �� 348.blk
    tpmms(&buf);

    // ����������ѡ���㷨���ҳ�S.C = 130��Ԫ��
    // 120 ��ʼ����
    // 111 - 116 ��������
    index_search(&buf, 130);


    // ������������Ӳ����㷨���������� S.C = R.A ��Ԫ��
    // 401 ��ʼ�����ӽ��
    sort_merge_join(&buf);

    // �������������ɨ���㷨�Ĳ���������
    // 140 ��������
    // 801 ��������
    // 901 ��������
    intersect(&buf);
    Union(&buf);
    subtract(&buf);
    return 0;
}
