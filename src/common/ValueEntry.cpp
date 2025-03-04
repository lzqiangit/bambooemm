#include "ValueEntry.hpp"
#include "KV.hpp"
ValueEntry::ValueEntry()
{
    this->len = 0;
    this->p = nullptr;
}

ValueEntry::ValueEntry(int len, char *p)
{
    this->len = 0;
    SetValue(len, p);
}

ValueEntry::ValueEntry(const ValueEntry &other)
{
    /** 拷贝构造函数中不能再传递同类对象,否则会造成递归调用,死循环 */
    // 深拷贝

    len = other.getLen();
    if (len != 0)
    {
        p = new char[len + 1];
        memcpy(p, other.getP(), len + 1);
    }
}

ValueEntry::ValueEntry(vector<KV> kvs)
{
    this->len = 0;
    for (KV kv : kvs)
    {
        char *kcv = kv.Splice();
        this->AppendValue(kcv);
    }
}

ValueEntry::ValueEntry(ValueEntry &&other) noexcept
    : len(other.len), p(other.p)
{
    other.p = nullptr; // 转移资源所有权
    other.len = 0;
}

void ValueEntry::SetValue(KV kv)
{
    this->len = 0;
    char *kcv = kv.Splice();
    this->AppendValue(kcv);
}

ValueEntry::~ValueEntry()
{
    if (len != 0)
    {
        delete[] p;
    }
}

void ValueEntry::SetValue(int len, char *p)
{
    if (this->len != 0)
    {
        delete[] this->p;
    }
    this->len = len;
    this->p = new char[len + 1];
    memcpy(this->p, p, len + 1);
}

/**
 * 将value设置为 key|counter|value
 */
void ValueEntry::SetValue(char *key, int counter, char *value)
{
    if (this->len > 0)
    {
        delete[] this->p;
        this->len = 0;
    }
    KV kv(key, counter, value);
    char *kcv = kv.Splice();
    this->p = kcv;
    this->len = strlen(kcv); //
}

void ValueEntry::SetValue(vector<char *> values)
{
    int num = values.size();
    int sumLen = 0;
    for (char *value : values)
    {
        sumLen += strlen(value);
    }
    sumLen += num; // num - 1 + 1 算上了 \0
    char *tempP = new char[sumLen];
    memset(tempP, 0, sumLen);

    sprintf(tempP, "%s", values[0]);
    int star = strlen(values[0]);
    for (int i = 1; i < num; i++)
    {
        char *value = values.at(i);
        int len = strlen(value);
        sprintf(tempP + star, ",%s", value);
        star += len;
    }

    if (this->len != 0)
    {
        delete[] p;
    }
    this->len = sumLen;
    this->p = tempP;
}

char *ValueEntry::DuplicateValue(int &len, char *dv)
{
    len = this->len;
    dv = new char[len];
    memset(dv, 0, len);
    memcpy(dv, this->p, len);
    return dv;
}

void ValueEntry::CpFrom(ValueEntry ve)
{
    if (len != 0)
    {
        delete[] this->p;
    }
    this->len = ve.getLen();
    this->p = new char[len];
    memset(this->p, 0, this->len);
    memcpy(this->p, ve.getP(), this->len);
}
/**
 * 在value后拼接字符串
 */
void ValueEntry::Append(char *append)
{
    string after;
    if (this->len > 0)
    {
        string val = p;
        string app = append;
        after = val + app;
    }
    else
    {
        after = append;
    }

    if (this->len > 0)
    {
        delete[] p;
    }
    this->len = after.length();
    p = new char[len + 1];
    memset(p, 0, len + 1);
    memcpy(p, (char *)after.c_str(), len);
}

void ValueEntry::AppendValue(char *append)
{

    string after;
    if (this->len > 0)
    {
        string val = p;
        string app = append;
        after = val + "," + app;
    }
    else
    {
        after = append;
    }
    if (this->len > 0)
    {
        delete[] p;
    }
    this->len = after.length();
    p = new char[len + 1];
    memset(p, 0, len + 1);
    memcpy(p, (char *)after.c_str(), len);
}

void ValueEntry::erase()
{
    if (len != 0)
    {
        len = 0;
        delete[] p;
    }
}

/**
 * 加密values
 */
bool ValueEntry::Enc(const char *password)
{

    int encLen = ((this->len + 15) / 16 + 1) * 16;
    char *encVals = new char[encLen];
    int retEncLen; // 调试无误可以删除！！！！！
    if (-1 == aes_encrypt_string(password, this->p, this->len, encVals, &retEncLen))
    {
        cout << "加密失败!" << endl;
    }
    if (encLen < retEncLen)
    {
        cout << "密文长度错误!" << endl;
        return false;
    }
    SetValue(retEncLen, encVals);
    delete[] encVals;
    return true;
}
/**
 * 解密values
 */
bool ValueEntry::Dec(const char *password)
{
    int decLen;
    char *decVals = new char[this->len];
    if (aes_decrypt_string(password, this->p, this->len, decVals, &decLen) == -1)
    {
        cout << "解密失败" << endl;
        return false;
    }
    SetValue(decLen, decVals);
    delete[] decVals;
    return true;
}

/**
 * 在values后面添加random
 */
void ValueEntry::SpliceRandom(int pre = 0)
{
    int next = pre;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, RANDOM_MAX);

    do
    {
        next = dis(gen);
    } while (pre == next);

    char *nextStr = new char[RANDOM_MAX_LEN + 1];
    memset(nextStr, 0, RANDOM_MAX_LEN + 1);
    sprintf(nextStr, "$%d", next);
    Append(nextStr);
}
/**
 * 剔除values后面的random
 */
int ValueEntry::DivRandom()
{
    string valueEStr = this->p;
    int end = valueEStr.find('$');
    string substring = valueEStr.substr(0, end);
    char *tempValue = copy_const_str(substring.c_str());
    this->SetValue(strlen(tempValue) + 1, tempValue);
    string randomStr = valueEStr.substr(end + 1, valueEStr.length());
    return atoi(randomStr.c_str());
}

/**
 * 以 逗号(,) 为标志, 分割values为value数组
 */
vector<char *> ValueEntry::DivValue()
{
    vector<char *> values;
    string valueEStr = this->p;
    int star = 0;
    int pre = -1;
    int end = valueEStr.find(',');
    while (end != string::npos)
    {
        string substring = valueEStr.substr(star, end - star);
        char *tempValue = copy_const_str(substring.c_str());
        values.push_back(tempValue);
        star = end + 1;
        pre = end;
        end = valueEStr.find(',', star);
    }
    string substring = valueEStr.substr(pre + 1, valueEStr.length());
    char *tempValue = copy_const_str(substring.c_str());
    values.push_back(tempValue);
    return values;
}

int ValueEntry::getLen() const
{
    return this->len;
}
char *ValueEntry::getP() const
{
    return this->p;
}

ValueEntry &ValueEntry::operator=(ValueEntry &ve)
{
    // 应该先判断是否有属性在堆区，如果有，需要先释放
    if (len != 0)
    {
        delete[] p;
        p = nullptr;
    }
    // 深拷贝
    len = ve.getLen();
    p = new char[ve.getLen()];
    memcpy(p, ve.getP(), len);
    // 实现链式编程
    return *this;
}

size_t ValueEntry::getMemOverhead()
{
    return sizeof(int) + sizeof(char *) + len * sizeof(char);
}

