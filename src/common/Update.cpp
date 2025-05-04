#include "Update.hpp"
#include "UpdateEntry.hpp"


Update::Update(char op, const char *value, int len)
{
    this->op = op;
    this->len = len;
    this->value = new char[len+1];
    memcpy(this->value, value, len);
    this->value[len] = '\0';
}

// 析构函数
Update::~Update()
{
    delete[] this->value;
}

UpdateEntry Update::toUpdateEntry(const char *password) const
{
    // 拼接字符串 op||value
    string str = string(1, op) + "|" + string(value);
    // 添加随机数
    str += "|" + to_string(rand() % RANDOM_MAX);
    // 加密
    int encLen = ((str.length() + 1 + 15) / 16 + 1) * 16;
    char *encVals = new char[encLen];
    int retEncLen;
    if (-1 == aes_encrypt_string(password, str.c_str(), str.length() + 1, encVals, &retEncLen))
    {
        cout << "加密失败!" << endl;
    }
    if (encLen < retEncLen)
    {
        cout << "密文长度错误!" << endl;
    }
    return UpdateEntry(encVals, encLen);
}

