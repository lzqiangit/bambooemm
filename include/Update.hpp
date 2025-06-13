#ifndef _UPDATE_
#define _UPDATE_
#include "KV.hpp"

class UpdateEntry;

/**
 * UpdateEntry 存储 OP,newValueEntry
 */
class Update
{
public:
    char op;
    int len;
    char *value;
public:

    Update();

    Update(char op, const char *value, int len);

    ~Update();

    // 拷贝构造函数
    Update(const Update &other) {
        op = other.op;
        len = other.len;
        value = new char[len + 1];
        memcpy(value, other.value, len);
        value[len] = '\0';
    }
    Update &operator=(const Update &other) {
        if (this != &other) {
            delete[] value;
            op = other.op;
            len = other.len;
            value = new char[len + 1];
            memcpy(value, other.value, len);
            value[len] = '\0';
        }
        return *this;
    }
    
    /**
     * @brief 拼接, 添加随机数, 加密。将UpdateEntry转换为Update
     * @param password 密码
     */
    UpdateEntry toUpdateEntry(const char *password) const;
};
#endif