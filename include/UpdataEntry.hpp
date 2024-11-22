#ifndef _UPDATA_ENTRY_
#define _UPDATA_ENTRY_
#include "ValueEntry.hpp"

/**
 * UpdateEntry 存储 OP,newValueEntry
 */
class UpdataEntry : public ValueEntry
{
private:
    /* data */
public:
    UpdataEntry(/* args */) {}
    UpdataEntry(int len, char *p, char op) {
        SetValue(len, p, op);
    }
    UpdataEntry(const ValueEntry& other, char op) {
        CpFrom(other, op);
    };
    UpdataEntry(const UpdataEntry& other) : ValueEntry(other) {}

    ~UpdataEntry() {
        
    }

    /**
     * 设置values值
     */
    void SetValue(int len, char *p, char op) {
        char *newP = new char[len + 1];
        memset(newP, 0, len + 1);
        memset(newP, op, 1);
        memcpy(newP + 1, p, len);
        this->len = len + 1;
        this->p = newP;
    }
    /**
     * 通过value数组设置valueEntry中的value值
     */
    void SetValue(vector<char*> values, char op) {
        int num = values.size();
        int sumLen = 0;
        for (char *value : values) {
            sumLen += strlen(value);
        }
        sumLen += num;  // num - 1 + 1 算上了 \0
        char *tempP = new char[sumLen + 1];
        memset(tempP, 0, sumLen);
        memset(tempP, op, 1);

        sprintf(tempP + 1, "%s", values[0]);
        int star = strlen(values[0]);
        for(int i=1; i<num; i++) {
            char *value = values.at(i);
            int len = strlen(value);
            sprintf(tempP + star, ",%s", value);
            star += len + 1;
        }

        if (this->len != 0) {
            delete[] p;
        }
        this->len = sumLen;
        this->p = tempP;
    }

    /**
     * 切割并返回valueEntry中存储的各个value,Div前需要剔除random！！！
     */
    char* DivValue() {

        // 暂时调整指针位置
        ++this->p;
        --this->len;

        // 调用父函数,获取返回值
        vector<char*> ret = ValueEntry::DivValue();
        
        if (ret.size() != 1) {
            cout << "分割UpdataValue出错!";
            exit(0);
        }

        --this->p;
        ++this->len; 

        return ret[0];
    }

    /**
     * 复制value和p
     * 返回长度
     * 并将值复制到vp指针指向位置(函数内申请空间)
     */
    char *DuplicateValue(int &len, char *dv) {
        ++this->p;
        --this->len;

        // 调用父函数,获取返回值
        char* ret = ValueEntry::DuplicateValue(len, dv);

        --this->p;
        ++this->len; 

        return ret;
    }

    void CpFrom(ValueEntry ve, char op) {
        SetValue(ve.getLen(), ve.getP(), op);
    }

    char DivOP() {
        return this->p[0];
    }

};
#endif