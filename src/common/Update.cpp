#include "Update.hpp"
#include "UpdateEntry.hpp"
Update::Update(const KV kv, char op) : KV(kv)
{
    this->op = op;
}
// 拷贝构造函数
Update::Update(const Update &other) : KV(other)
{
    this->op = other.op;
}

Update::Update(char op, const char *key, int counter, const char *value) : KV(key, counter, value)
{
    this->op = op;
}

// 析构函数
Update::~Update()
{
}

/**
 * 将含有随机数的UpdateEntry转化为Update
 */
Update Update::ResolveFromUpdateEntry(const UpdateEntry &ue)
{
    char *left = strtok(ue.p, "$");
    char *key = strtok(left + 1, "|");
    char *counter = strtok(nullptr, "|");
    char *value = strtok(nullptr, "|");
    return Update(ue.p[0], key, atoi(counter), value);
}

/**
 * 将含有随机数的加密UpdateEntry列表转化为Update列表
 */
vector<Update> Update::ResolveFromUpdateEntries(vector<UpdateEntry> updateList, const char* password)
{
    vector<Update> ret;
    for (UpdateEntry &ue : updateList)
    {
        ue.Dec(password);
        ret.push_back(ResolveFromUpdateEntry(ue));
    }
    return ret;
}
