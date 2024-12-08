#include "utils.hpp"

vector<int> LoadVolumn() {
    MYSQL *con = NULL;
    con = mysql_init(con);
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "127.0.0.1";    
    unsigned int Port = 3306;  
    string User = "lzq";
    string PassWord = "0000";
    string DBName = "kvlist";
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }


    mysql_query(con, "select count(*) from random group by `key`");

    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    int nums = 0;  
    nums = mysql_num_fields(res); 

    MYSQL_FIELD * fields;
    vector<int> volumeList;

    int tempL = 0;
    char *bkey = nullptr;
    while( (row = mysql_fetch_row(res)) != nullptr) 
    {
        char *volum = new char[(strlen(row[0]) + 1)];
    
        strcpy(volum, row[0]);

        volumeList.push_back(atoi(volum));
    }
    mysql_free_result(res);
    mysql_close(con);
    return volumeList;
}


int LenOfInt(int num) {
    int len = 1;
    while (num >= 10) {
        ++len;
        num /= 10;
    }
    return len;
}

int LenOfUInt(uint32_t num) {
    int len = 1;
    while (num >= 10) {
        ++len;
        num /= 10;
    }
    return len;
}

void GenKey(int level) {
    // string str = "very nice day!";
    string keyStr = "testkey";
    const char* key = keyStr.c_str();      // (unsigned char*)str.c_str();// AESGen(level);
    ofstream outFile("/home/lzq/bemmkey/KI.bin", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error writing file." << std::endl;
        return;
    }

    outFile.write((char*)key, strlen((char*)key));
    outFile.close();
}

char* LoadKey() {

    std::ifstream file("/home/lzq/bemmkey/KI.bin", std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        char* buffer = new char[fileSize + 1];
        memset(buffer, 0, fileSize + 1);
        if (file.read( (char*) buffer, fileSize)) {
            file.close();
            return buffer;
        } else {
            std::cerr << "Error reading file." << std::endl;
            file.close();
            delete[] buffer;
            return nullptr;
        }
    } else {
        std::cerr << "Unable to open file for reading." << std::endl;
        return nullptr;
    }
}

void printBinary(char* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        for (int j = 7; j >= 0; --j) {
            std::cout << ((data[i] >> j) & 1);
        }
        std::cout << " "; //
    }
    std::cout << endl;
    
}

uint32_t get_value_id(const char* value) {  
    string input = value;
    // 
    std::size_t pos = input.rfind('_');  
    if (pos == std::string::npos || pos == input.size() - 1) {  
        throw std::invalid_argument("Invalid input string format");  
    }  
      
    std::string idStr = input.substr(pos + 1);  
    // 
    uint32_t id;  
    std::istringstream iss(idStr);  
    if (!(iss >> id)) {  
        throw std::invalid_argument("Failed to convert id to unsigned int");  
    }  
      
    return id;  
}

void print_uint64(uint64_t num) {
    bitset<sizeof(unsigned long int) * 8> binary(num);  
    std::cout << binary << std::endl;  
}

void print_64title() {
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            cout << j;
        }
    }
    cout << endl;
}

char* copy_const_str(const char* cstr) {
    int len = strlen(cstr);
    char* cpy = new char[len + 1];
    memset(cpy, 0, len + 1);
    memcpy(cpy, cstr, len);
    return cpy;
}

/*
*****************************************************************************************
*   函 数 名: aes_encrypt_string
*   功能说明: AES加密字符串
*   形    参:   _pPassword  :   密码
*               _pInput     :   输入数据
*               _InLen      :   输入数据长度
*               _pOutBuf    :   输出AES编码数据
*               _pOutLen    :   输出AES编码数据长度
*   返 回 值: 0：成功, -1：失败
*   注意: 传入的指针需要提前申请空间,否则会报Segmentation fault
*****************************************************************************************
*/
int aes_encrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen)
{
    // 上下文结构
    EVP_CIPHER_CTX *pEn_ctx = NULL;

    int ret = -1;
    int flen = 0, outlen = 0;
    int i, nrounds = 1;

    // 存储秘钥和初始化向量
    unsigned char key[32] = {};
    unsigned char iv[32] = {};

    // 参数判断
    if (_pPassword == NULL || _pInput == NULL || _pOutBuf == NULL || _pOutLen == NULL)
    {
        return ret;
    }

    // 设置使用 256 位密钥长度的 AES 加密算法，并采用 CBC 模式。
    const EVP_CIPHER *cipherType = EVP_aes_256_cbc();
    if (cipherType == NULL)
    {
        goto clean;
    }

    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    // 通过输入密码产生密钥key和初始化向量iv
    i = EVP_BytesToKey(cipherType, EVP_md5(), NULL, (unsigned char *)_pPassword, strlen(_pPassword), nrounds, key, iv);
    if (i != 32)
    {
        printf("Key size is %d bits - should be 256 bits\n", i);
        goto clean;
    }

    pEn_ctx = EVP_CIPHER_CTX_new();                         // 创建加密上下文
    EVP_CIPHER_CTX_init(pEn_ctx);                           // 初始化 EVP_CIPHER_CTX 上下文
    EVP_EncryptInit_ex(pEn_ctx, cipherType, NULL, key, iv); // 初始化加密操作

    /* Update cipher text */
    if (!EVP_EncryptUpdate(pEn_ctx, (unsigned char *)_pOutBuf, &outlen, (unsigned char *)_pInput, _InLen))
    { // 处理数据
        cout << "Error,ENCRYPR_UPDATE:" << endl;
        goto clean;
    }

    /* updates the remaining bytes */
    if (!EVP_EncryptFinal_ex(pEn_ctx, (unsigned char *)(_pOutBuf + outlen), &flen))
    { // 完成加密操作，处理剩余字节
        cout << "Error,ENCRYPT_FINAL:" << endl;
        goto clean;
    }

    *_pOutLen = outlen + flen;

    ret = 0; /* SUCCESS */

clean:
    // 清理内存
    if (pEn_ctx)
        EVP_CIPHER_CTX_cleanup(pEn_ctx);
    if (pEn_ctx)
        EVP_CIPHER_CTX_free(pEn_ctx);

    return ret;
}

/*
*****************************************************************************************
*   函 数 名: aes_decrypt_string
*   功能说明: AES解密得到字符串
*   形    参:   _pPassword  :   密码
*               _pInput     :   输入需解密的数据
*               _InLen      :   输入需解密的数据长度
*               _pOutBuf    :   输出AES解密后的字符串
*               _pOutLen    :   输出AES编码数据长度
*   返 回 值: 0：成功, -1：失败
*****************************************************************************************
*/
int aes_decrypt_string(char *_pPassword, char *_pInput, int _InLen, char *_pOutBuf, int *_pOutLen)
{
    // 上下文结构
    EVP_CIPHER_CTX *pDe_ctx = NULL;

    int ret = -1;
    int flen = 0, outlen = 0;
    int i, nrounds = 1;

    // 存储秘钥和初始化向量
    unsigned char key[32] = {};
    unsigned char iv[32] = {};

    // 参数判断
    if (_pPassword == NULL || _pInput == NULL || _pOutBuf == NULL || _pOutLen == NULL)
    {
        return ret;
    }

    // 设置使用 256 位密钥长度的 AES 加密算法，并采用 CBC 模式。
    const EVP_CIPHER *cipherType = EVP_aes_256_cbc();
    if (cipherType == NULL)
    {
        goto clean;
    }

    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    // 通过输入密码产生密钥key和初始化向量iv
    i = EVP_BytesToKey(cipherType, EVP_md5(), NULL, (unsigned char *)_pPassword, strlen(_pPassword), nrounds, key, iv);
    if (i != 32)
    {
        printf("Key size is %d bits - should be 256 bits\n", i);
        goto clean;
    }

    pDe_ctx = EVP_CIPHER_CTX_new();                         // 创建加密上下文
    EVP_CIPHER_CTX_init(pDe_ctx);                           // 初始化 EVP_CIPHER_CTX 上下文
    EVP_DecryptInit_ex(pDe_ctx, cipherType, NULL, key, iv); // 初始化解密操作

    /* Update cipher text */
    if (!EVP_DecryptUpdate(pDe_ctx, (unsigned char *)_pOutBuf, &outlen, (unsigned char *)_pInput, _InLen))
    { // 处理数据
        cout << "Error,DEC_UPDATE:" << _pOutBuf << endl;
        goto clean;
    }

    /* updates the remaining bytes */
    if (EVP_DecryptFinal_ex(pDe_ctx, (unsigned char *)(_pOutBuf + outlen), &flen) != 1)
    { // 完成解密操作，处理剩余字节
        cout << "Error,DEC_FINAL!" << _pOutBuf << "|" << flen << endl;
        goto clean;
    }

    *_pOutLen = outlen + flen;

    ret = 1;
clean:
    // 清理内存
    if (pDe_ctx)
        EVP_CIPHER_CTX_cleanup(pDe_ctx);
    if (pDe_ctx)
        EVP_CIPHER_CTX_free(pDe_ctx);

    return ret;
}

vector<KV *> LoadKVList(int &n, int &l) {
    MYSQL *con = NULL;
    con = mysql_init(con);
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "127.0.0.1";    
    unsigned int Port = 3306;   
    string User = "lzq";   
    string PassWord = "0000";  
    string DBName = "kvlist"; 
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }

    mysql_query(con, "select * from random");

    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    int nums = 0;  
    nums = mysql_num_fields(res);  

    MYSQL_FIELD * fields;
    vector<KV *> kvList;
    n = 0;
    l = 0;
    int tempL = 0;
    char *bkey = nullptr;
    while( (row = mysql_fetch_row(res)) != nullptr)
    {
        ++n;
        char *key = new char[(strlen(row[0]) + 1)];
        char *value = new char[(strlen(row[1]) + 1)];
        strcpy(key, row[0]);
        strcpy(value, row[1]);

        if(bkey == nullptr) {
            bkey = key;
        }

        if ( strcmp(key, bkey) != 0 ) {
            l = max(l, tempL);
            tempL = 1;
            bkey = key;
        } else {
            ++tempL;
        }
        


        KV *kv = new KV(key, value, stoi(string(row[2])));
        kvList.push_back(kv);
    }
    mysql_free_result(res);
    mysql_close(con);
    return kvList;
}


uint32_t GetYHash(uint32_t x, uint32_t st1) {
    string st1Str = to_string(st1);
    string xStr = to_string(x);
    string splice = xStr + "|" + st1Str;
    char *ret = new char[splice.length() + 1];
    memset(ret, 0, splice.length() + 1);
    memcpy(ret, (char*)splice.c_str(), splice.length());
    uint32_t retU32 = BOBHash::run(ret, splice.length() + 1, 3);
    return retU32;
}

uint64_t getTimestamp() {
    time_t now = time(nullptr);
    return static_cast<uint64_t>(now) * 1000;
}