#ifndef _UPDATE_ENTRY_
#define _UPDATE_ENTRY_

#include "ValueEntry.hpp"
class Update;

class UpdateEntry
{
public:
    char* value;
    int len;

public:
    // 拷贝构造函数
    UpdateEntry(const char *value, int len);

    /**
     * @brief 将UpdateEntry转换为Update
     * @param password 密码
     */
    Update toUpdate(const char* password);

    size_t getMemOverhead();
};

#endif