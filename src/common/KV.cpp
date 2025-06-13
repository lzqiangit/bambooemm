#include "KV.hpp"

KV::KV()
{
    key = nullptr;
    value = nullptr;
    counter = 0;
}

/**
 * 构建一个KV，传入key,counter和value
 * 注意构造kv后后对key和value的释放
 */
KV::KV(const char *key, int counter, const char *value)
{
    this->key = strdup(key);
    this->value = strdup(value);
    this->counter = counter;
}

/**
 * 构建一个填充KV
 */
KV::KV(const char *key, int counter)
{
    this->key = strdup(key);
    this->counter = counter;
    this->value = nullptr;
    BePadding();
}

/**
 * 移动构造函数
 */
KV::KV(KV &&others)
{

    this->key = others.key;
    this->value = others.value;
    this->counter = others.counter;
    others.key = nullptr;
    others.value = nullptr;
    others.counter = 0;
}

KV::~KV()
{
    delete key;
    delete value;
}

/**
 * 从key|counter|val的字符串中导入kv
 * 首先以 | 分割，key为第一个，counter为第二个，value为第三个
 * 注意counter需要转化为整形
 */
KV::KV(char *kcv)
{
    char *key = strtok(kcv, "|");
    char *counter = strtok(nullptr, "|");
    char *value = strtok(nullptr, "|");
    this->key = strdup(key);
    if (value == nullptr)
    {
        this->value = new char[2];
        this->value[0] = 'P';
        this->value[1] = '\0';
        return;
    }
    this->value = strdup(value);
    this->counter = atoi(counter);
}

/**
 * 拷贝构造函数
 * 深拷贝
 */
KV::KV(const KV &others)
{
    // cout << "CopyConstruct" << "\n";
    this->key = strdup(others.key);
    this->value = strdup(others.value);
    this->counter = others.counter;
}

KV &KV::operator=(const KV &others)
{
    if (this->key != nullptr)
    {
        delete[] this->key;
    }
    if (this->value != nullptr)
    {
        delete[] this->value;
    }
    this->key = strdup(others.key);
    this->value = strdup(others.value);
    this->counter = others.counter;
    return *this;
}

/**
 * 将成员变量以 key|counter|value 的形式拼接成字符串
 */
char *KV::Splice() const
{
    string counterStr = to_string(this->counter);
    return concat('|', this->key, counterStr.c_str(), this->value, NULL);
}

/**
 * 获取其向服务端发送查询请求所需的key <- hash(key)|counter
 */
char *KV::QueryKey(uint32_t K)
{
    return MakeSearchKey(MakeHashKey(this->key), this->counter);
}

/**
 * 设置为填充值
 */
void KV::BePadding()
{
    if (this->value != nullptr)
        delete[] this->value;
    this->value = strdup("P");
}

bool KV::isPadding()
{
    return (strlen(this->value) == 1 && this->value[0] == 'P');
}

void KV::setValue(const char *newValue)
{
    if (this->value != nullptr)
    {
        delete[] this->value;
    }
    this->value = strdup(newValue);
}

/**
 * 求key的BobHash哈希值
 */
string KV::MakeHashKey(const char *key)
{

    uint32_t hash_key = BOBHash::run(key, strlen(key), 3);
    string keyStr = to_string(hash_key);
    return keyStr;
}
/**
 * 传入哈希后的key和counter,输出两者的拼接
 * 配合 KV::MakeHashKey 生成key的哈希值string
 * key <- hash(k)||c
 */
char *KV::MakeSearchKey(string hashKey, int c)
{

    string counterStr = to_string(c);
    string keyCounterStr = hashKey + "|" + counterStr;
    int len = keyCounterStr.length();
    char *retCStr = new char[len + 1];
    memset(retCStr, 0, len + 1);
    memcpy(retCStr, (char *)keyCounterStr.c_str(), len);
    return retCStr;
}
/**
 * 从key||counter||val的字符串列表中导入kv的列表
 */
vector<KV> KV::LoadKVList(vector<char *> kvStrList)
{
    vector<KV> ret;
    for (char *kvStr : kvStrList)
    {
        // 包含'|'的字符串
        if (strchr(kvStr, '|') == nullptr)
        {
            continue;
        }
        KV kv(kvStr);
        ret.push_back(kv);
    }
    return ret;
}

size_t KV::getMemOverhead() const
{
    // 计算消耗空间的大小
    size_t overhead = 0;
    overhead += sizeof(KV); // KV对象本身的大小
    overhead += strlen(this->key);
    overhead += strlen(this->value);
    overhead += sizeof(int); // counter的大小
    return overhead;
}

char *KV::copy_const_str(const char *cstr)
{
    int len = strlen(cstr);
    char *cpy = new char[len + 1];
    memset(cpy, 0, len + 1);
    memcpy(cpy, cstr, len);
    return cpy;
}
int KV::LenOfInt(int num)
{
    int len = 1;
    while (num >= 10)
    {
        ++len;
        num /= 10;
    }
    return len;
}
