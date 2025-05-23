#ifndef FULL_BINARY_TREE_H_
#define FULL_BINARY_TREE_H_

#include "ValueEntry.hpp"

/**
 * @brief 完全二叉树
 * 节点编号从0开始
 */
class FullBinaryTree
{
private:
    // ValueEntry数组
    ValueEntry *mValueEntries;
    int mH;                  // 高度
    int mLeafCount;          // 叶子节点个数
    int mNodeCount;          // 总节点个数
    int mFirstLeftIndex;     // 第一个左子节点的索引
    uint32_t mLeftIndexMask; // 叶子点索引的掩码

public:
    FullBinaryTree()
    {
    }
    FullBinaryTree(int height)
    {
        setup(height);
    }
    ~FullBinaryTree()
    {
        delete[] mValueEntries;
        mValueEntries = nullptr;
    }

    /**
     * 初始化
     * @param height 高度
     */
    void setup(int height)
    {
        mNodeCount = (1 << height) - 1;

        mLeafCount = 1 << (height - 1);

        mFirstLeftIndex = mLeafCount - 1;

        mLeftIndexMask = (1 << (height - 1)) - 1;

        mValueEntries = new ValueEntry[mNodeCount];
    }

    /**
     * 通过元素的hash值插入元素
     * @param hashValue 元素的hash值b
     * @param ve 元素
     * @return 如果插入成功, 返回true, 否则返回false
     */
    bool InsertByHashValue(uint32_t b, const ValueEntry &ve)
    {
        // 计算hash值对应的叶子节点的索引
        int index = (b & mLeftIndexMask) + mFirstLeftIndex;
        return Insert(index, ve);
    }

    /**
     * 通过元素的hash值查询元素
     * @param b 元素的hash值
     * @return 查询到的元素
     */
    vector<ValueEntry> QueryByHashValue(uint32_t b) const
    {
        int index = (b & mLeftIndexMask) + mFirstLeftIndex;
        return GetValueEntries(index);
    }

    void ClearByHashValue(int b) {
        int index = (b & mLeftIndexMask) + mFirstLeftIndex;
        if (index < mFirstLeftIndex)
        {
            cout << "非叶子节点, 不能进行清空！" << endl;
            return;
        }
        while (index >= 0)
        {
            mValueEntries[index].erase();
            if (index == 0) break;
            index = (index - 1) / 2;
        }
    }
private:
    /**
     * 获取叶节点index到根节点的路径上的所有元素
     * @param index 叶节点的索引
     * @return 叶节点到根节点路径上的所有元素, 如果传入的不是叶子节点, 返回空
     */
    vector<ValueEntry> GetValueEntries(int index) const
    {
        vector<ValueEntry> ret;
        if (index < mFirstLeftIndex)
        {
            cout << "非叶子节点, 不能进行查询！" << endl;
            return ret;
        }
        while (index >= 0)
        {
            ret.push_back(mValueEntries[index]);
            if (index == 0) break;
            index = (index - 1) / 2;
        }
        return ret;
    }

    /**
     * 向完全二叉树中插入元素, 传入一个叶子节点的编号, 从这个叶子节点向上开始,将元素插入到第一个非空节点
     * @param index 插入的索引, 必须是叶子节点
     * @param ve 插入的元素
     * @return 如果传入的不是叶子节点, 或者根节点到叶子节点路径上没有空节点,返回false
     */
    bool Insert(int index, const ValueEntry &ve)
    {
        // 判断是否是叶子节点, 不是就返回false, 紧张从非叶子节点插入元素
        if (index < mFirstLeftIndex)
        {
            cout << "非叶子节点, 不能插入！" << endl;
            return false;
        }
        // 如果是叶子节点, 向上找到第一个非空节点插入

        while (index >= 0)
        {
            if (mValueEntries[index].isEmpty())
            {
                // 插入
                mValueEntries[index].CpFrom(ve);
                return true;
            }
            if (index == 0) break;
            index = (index - 1) / 2;
        }
        
        return false;
    }


};

#endif