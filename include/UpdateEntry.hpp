#ifndef _UPDATE_ENTRY_
#define _UPDATE_ENTRY_

#include "ValueEntry.hpp"
class Update;

class UpdateEntry : public ValueEntry
{

public:
    // 拷贝构造函数
    UpdateEntry(const UpdateEntry &other);

    UpdateEntry(const Update &update);

    UpdateEntry(const KV &kv, char op);

    ~UpdateEntry() override;

    // 重载等于运算符
    UpdateEntry &operator=(const UpdateEntry &other);
};

#endif