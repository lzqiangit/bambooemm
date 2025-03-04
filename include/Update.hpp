#ifndef _UPDATE_
#define _UPDATE_
#include "KV.hpp"

class UpdateEntry;

/**
 * UpdateEntry 存储 OP,newValueEntry
 */
class Update : public KV
{
public:
    char op;
    Update(const KV kv, char op);
    // 拷贝构造函数
    Update(const Update &other);
    Update(char op, const char* key, int counter, const char* value);
    ~Update();
    static Update ResolveFromUpdateEntry(const UpdateEntry& ue);
    static vector<Update> ResolveFromUpdateEntries(vector<UpdateEntry> updateList, const char* password);
};
#endif