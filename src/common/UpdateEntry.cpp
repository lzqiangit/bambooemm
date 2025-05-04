#include "UpdateEntry.hpp"
#include "Update.hpp"
#include "ValueEntry.hpp"
#include "KV.hpp"



// UpdateEntry::UpdateEntry(const UpdateEntry &other) : ValueEntry(other)
// {
// }

// /**
//  * 将KV和op拼接为UpdateEntry, 并附加上伪随机数random
//  */
// UpdateEntry::UpdateEntry(const Update &update)
// {
//     int random = rand() % RANDOM_MAX;
//     const char *randomStr = to_string(random).c_str();
//     const char *counterStr = to_string(update.counter).c_str();

//     int keyLen = strlen(update.key);
//     int counterLen = strlen(counterStr);
//     int valueLen = strlen(update.value);
//     int randomLen = strlen(randomStr);
//     int sumLen = keyLen + counterLen + valueLen + randomLen + 5;
//     // 初始化指针
//     this->p = new char[sumLen];
//     memset(this->p, 0, sumLen);
//     this->len = sumLen;

//     this->p[0] = update.op;
//     memcpy(this->p + 1, update.key, strlen(update.key));
//     this->p[keyLen + 1] = '|';

//     memcpy(this->p + keyLen + 2, counterStr, counterLen);
//     this->p[keyLen + 2 + counterLen] = '|';
//     memcpy(this->p + keyLen + 2 + counterLen + 1, update.value, valueLen);

//     this->p[keyLen + 2 + counterLen + 1 + valueLen] = '$';
//     memcpy(this->p + keyLen + 2 + counterLen + 1 + valueLen + 1, randomStr, randomLen);
// }

// UpdateEntry::UpdateEntry(const KV &kv, char op) : UpdateEntry(Update(kv, op)) {}

// UpdateEntry::~UpdateEntry() {

// }

// // 重载等于运算符
// UpdateEntry &UpdateEntry::operator=(const UpdateEntry &other)
// {
//     if (this->len != 0)
//     {
//         delete[] this->p;
//     }
//     this->len = other.len;
//     this->p = new char[len];
//     memcpy(this->p, other.p, len);
//     return *this;
// }

UpdateEntry::UpdateEntry(const char *value, int len)
{
    this->len = len;
    this->value = new char[len + 1];
    memset(this->value, 0, len + 1);
    memcpy(this->value, value, len);
}

Update UpdateEntry::toUpdate(const char *password)
{
    // 解密，分割随机数，op和value
    int decLen;
    char *decVals = new char[this->len];
    if (aes_decrypt_string(password, this->value, this->len, decVals, &decLen) == -1)
    {
        cout << "解密失败" << endl;
    }
    // decVals 按照 op|value|random 拼接, 解析出这三部分
    string decStr = decVals;
    int opEnd = decStr.find('|');
    char op = decStr[0];
    string valueStr = decStr.substr(opEnd + 1, decStr.length());
    int randomEnd = valueStr.find('|');
    valueStr = valueStr.substr(0, randomEnd);
    int end = valueStr.find('|');
    valueStr = valueStr.substr(0, end);
    return Update(op, valueStr.c_str(), valueStr.length());
}

size_t UpdateEntry::getMemOverhead()
{
    return 1;
}
