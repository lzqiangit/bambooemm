#include "utils.hpp"
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>  
#include <string>  
#include <stdexcept> 
#include <bitset>   
using namespace std;

vector<KV *> LoadKVList(int &n, int &l) {
    MYSQL *con = NULL;
    con = mysql_init(con);//初始化
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "localhost";    //主机地址
    unsigned int Port = 3306;   //数据库端口号
    string User = "lzq";   //登陆数据库用户名
    string PassWord = "0000";  //登陆数据库密码
    string DBName = "kvlist"; //使用数据库名
    //链接数据库
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }

    //执行sql语句，如果查询成功，mysql_query()函数会返回0；否则，返回非零值表示发生错误。
    mysql_query(con, "select * from kv_list_10_5");

    MYSQL_RES *res;
    MYSQL_ROW row;
    //获得执行结果
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    //获取字段个数，即查询获得的结果里有几列数据
    int nums = 0;  
    nums = mysql_num_fields(res);  //属于表结构的获取

    MYSQL_FIELD * fields;
    vector<KV *> kvList;
    n = 0;
    l = 0;
    int tempL = 0;
    char *bkey = nullptr;
    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()函数从指定的结果集中获取一行数据返回给row，是数组的形式，即row内部是字符串数组指针（二级指针）
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

unsigned char* AESGen(int keyLength) {
    unsigned char *key = new unsigned char[keyLength+1];
    if(!RAND_bytes(key, keyLength)) {
        cout << "Failed to generate a random key!" << endl;
    }
    return key;
}

unsigned char* AESEnc(const unsigned char *key, const unsigned char *plaintext) {

    size_t length = strlen(reinterpret_cast<const char*>(plaintext));
    unsigned char* ciphertext = new unsigned char[length + 1];
    memset(ciphertext, 0, length+1);
    AES_KEY encryptKey;
    AES_set_encrypt_key(key, strlen((char*)key), &encryptKey);
    AES_encrypt(plaintext, ciphertext, &encryptKey);
    return ciphertext;
}

unsigned char* AESDec(const unsigned char *key, const unsigned char *ciphertext) {
    size_t length = strlen(reinterpret_cast<const char*>(ciphertext));
    unsigned char* plaintext = new unsigned char[length + 1];
    AES_KEY decryptKey;
    AES_set_decrypt_key(key, strlen((char*)key), &decryptKey);
    AES_decrypt(ciphertext, plaintext, &decryptKey);
    return plaintext;
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
    unsigned char* key = AESGen(level);      // (unsigned char*)str.c_str();// AESGen(level);
    ofstream outFile("/home/lzq/bemmkey/KI.bin", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error writing file." << std::endl;
        return;
    }

    outFile.write((char*)key, strlen((char*)key));
    outFile.close();
}

unsigned char* LoadKey() {

    std::ifstream file("/home/lzq/bemmkey/KI.bin", std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        unsigned char* buffer = new unsigned char[fileSize + 1];
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
        std::cout << " "; // 每个字节之间加一个空格
    }
    std::cout << endl;
}

uint32_t get_value_id(const char* value) {  
    string input = value;
    // 使用字符串流和字符串操作找到最后一个下划线并提取 id 部分  
    std::size_t pos = input.rfind('_');  
    if (pos == std::string::npos || pos == input.size() - 1) {  
        throw std::invalid_argument("Invalid input string format");  
    }  
      
    std::string idStr = input.substr(pos + 1);  
      
    // 使用字符串流将 id 部分转换为 unsigned int  
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
