#ifndef TWO_CH_H_
#define TWO_CH_H_

#include "FullBinaryTree.hpp"
#include <cmath>
#include <xxhash.h>
/**
 * 2CH_FB方案
 */
class TwoCH
{
private:
    FullBinaryTree *mFullBinaryTree; // 完全二叉树

    int mCapicity;   // 最大容量
    int mSTreeNum;    // 子树的数量
    int mSTreeHeight; // 子树的高度
    int mMaxVolume;   // 最大容量

    const int TCH_C = 1; // 参数c

public:
    /**
     * @brief 构造函数
     * @param n 总容量
     */
    TwoCH(int n)
    {
        mCapicity = n;
        mSTreeNum = ceil((double)n / (TCH_C * log(n)));
        mSTreeHeight = ceil(log(TCH_C * log(n)));

        // 创建子树mSTreeNum棵子树
        mFullBinaryTree = new FullBinaryTree[mSTreeNum];
        for (int i = 0; i < mSTreeNum; i++)
        {
            mFullBinaryTree[i].setup(mSTreeHeight);
        }
    }
    /**
     * 析构函数
     */
    ~TwoCH()
    {
        delete[] mFullBinaryTree;
    }

    /**
     * @brief 设置最大容量
     */
    void setMaxVolume(int maxVolume)
    {
        mMaxVolume = maxVolume;
    }

    /**
     * @brief 插入元素
     * @param FK 关键字的哈希值
     * @param j 元素的counter
     * @param valueE 元素
     */
    bool Insert(uint32_t FK, int j, const ValueEntry& valueE) {
       
        for (int i=0; i<=1; i++) {
            // 级联j||i计算哈希,
            const char* concat = concatInt(j, i); 
            int len = strlen(concat);
            uint32_t b = XXH32(concat, len, FK);
            // 计算树编号和叶子节点编号
            /**
             * 32位的哈希值, 其中, 树的高度固定, 所以每一棵树的叶子节点固定
             * 主存储结构拥有的树的数量也是可知的
             * 低(mSTreeHeight-1)位表示子树的叶节点编号
             * 高clie(log(mSTreeNum))表示子树编号
             */
            int treeIndex = (b >> (mSTreeHeight - 1)) % mSTreeNum; // 子树编号
            if (mFullBinaryTree[treeIndex].InsertByHashValue(b, valueE)) {
                return true;
            }
        }
        //TODO 插入失败需要添加到溢出栈之中
        return false;
    }

    /**
     * @brief 查询元素
     * @param key 关键字的哈希值
     * @return 查询结果
     */
    vector<ValueEntry> Query(uint32_t FK) {
        vector<ValueEntry> result;
        for (int i=0; i<=1; i++) {
            // 级联j||i计算哈希,
            // 遍历最大容量
            for (int j=0; j<mMaxVolume; j++) {
                const char* concat = concatInt(j, i); 
                int len = strlen(concat);
                uint32_t b = XXH32(concat, len, FK);
                // 计算树编号和叶子节点编号
                /**
                 * 32位的哈希值, 其中, 树的高度固定, 所以每一棵树的叶子节点固定
                 * 主存储结构拥有的树的数量也是可知的
                 * 低(mSTreeHeight-1)位表示子树的叶节点编号
                 * 高clie(log(mSTreeNum))表示子树编号
                 */
                int treeIndex = (b >> (mSTreeHeight - 1)) % mSTreeNum; // 子树编号
                vector<ValueEntry> ve = mFullBinaryTree[treeIndex].QueryByHashValue(b);
                for (auto v : ve) {
                    result.push_back(v);
                }
            }
            
        }
        //TODO 插入失败需要添加到溢出栈之中
        return result;
    }


private:
    const char* concatInt(int a, int b) {
        string str = to_string(a) + "|" + to_string(b);
        return copy_const_str(str.c_str());
    }
};

#endif