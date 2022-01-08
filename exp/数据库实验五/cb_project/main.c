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

    // 使用线性搜索算法，找出S.C = 130的元组
    // 100 开始存线性查找结果
    linear_search(&buf, 130);

    // 两阶段多路归并排序算法
    // 201 - 248 放分组排序后的临时文件
    // 关系 R 排序后输出到文件 301.blk 到 316.blk
    // 关系 S 排序后输出到文件 317.blk 到 348.blk
    tpmms(&buf);

    // 基于索引的选择算法，找出S.C = 130的元组
    // 120 开始存结果
    // 111 - 116 存索引块
    index_search(&buf, 130);


    // 基于排序的连接操作算法，连接所有 S.C = R.A 的元组
    // 401 开始存连接结果
    sort_merge_join(&buf);

    // 基于排序的两趟扫描算法的并交差运算
    // 140 交运算结果
    // 801 并运算结果
    // 901 差运算结果
    intersect(&buf);
    Union(&buf);
    subtract(&buf);
    return 0;
}
