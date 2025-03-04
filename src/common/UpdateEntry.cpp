#include "UpdateEntry.hpp"
#include "Update.hpp"
#include "ValueEntry.hpp"
#include "KV.hpp"
UpdateEntry::UpdateEntry(const UpdateEntry &other) : ValueEntry(other)
{
}

/**
 * 将KV和op拼接为UpdateEntry, 并附加上伪随机数random
 */
UpdateEntry::UpdateEntry(const Update &update)
{
    int random = rand() % RANDOM_MAX;
    const char *randomStr = to_string(random).c_str();
    const char *counterStr = to_string(update.counter).c_str();

    int keyLen = strlen(update.key);
    int counterLen = strlen(counterStr);
    int valueLen = strlen(update.value);
    int randomLen = strlen(randomStr);
    int sumLen = keyLen + counterLen + valueLen + randomLen + 5;
    // 初始化指针
    this->p = new char[sumLen];
    memset(this->p, 0, sumLen);
    this->len = sumLen;

    this->p[0] = update.op;
    memcpy(this->p + 1, update.key, strlen(update.key));
    this->p[keyLen + 1] = '|';

    memcpy(this->p + keyLen + 2, counterStr, counterLen);
    this->p[keyLen + 2 + counterLen] = '|';
    memcpy(this->p + keyLen + 2 + counterLen + 1, update.value, valueLen);

    this->p[keyLen + 2 + counterLen + 1 + valueLen] = '$';
    memcpy(this->p + keyLen + 2 + counterLen + 1 + valueLen + 1, randomStr, randomLen);
}

UpdateEntry::UpdateEntry(const KV &kv, char op) : UpdateEntry(Update(kv, op)) {}

UpdateEntry::~UpdateEntry() {

}

// 重载等于运算符
UpdateEntry &UpdateEntry::operator=(const UpdateEntry &other)
{
    if (this->len != 0)
    {
        delete[] this->p;
    }
    this->len = other.len;
    this->p = new char[len];
    memcpy(this->p, other.p, len);
    return *this;
}
