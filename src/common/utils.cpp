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
    con = mysql_init(con);//��ʼ��
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "127.0.0.1";    //������ַ
    unsigned int Port = 3306;   //���ݿ�˿ں�
    string User = "lzq";   //��½���ݿ��û���
    string PassWord = "0000";  //��½���ݿ�����
    string DBName = "kvlist"; //ʹ�����ݿ���
    //�������ݿ�
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }

    //ִ��sql��䣬�����ѯ�ɹ���mysql_query()�����᷵��0�����򣬷��ط���ֵ��ʾ��������
    mysql_query(con, "select * from random");

    MYSQL_RES *res;
    MYSQL_ROW row;
    //���ִ�н��
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    //��ȡ�ֶθ���������ѯ��õĽ�����м�������
    int nums = 0;  
    nums = mysql_num_fields(res);  //���ڱ��ṹ�Ļ�ȡ

    MYSQL_FIELD * fields;
    vector<KV *> kvList;
    n = 0;
    l = 0;
    int tempL = 0;
    char *bkey = nullptr;
    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()������ָ���Ľ�����л�ȡһ�����ݷ��ظ�row�����������ʽ����row�ڲ����ַ�������ָ�루����ָ�룩
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

vector<int> LoadVolumn() {
    MYSQL *con = NULL;
    con = mysql_init(con);//��ʼ��
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "127.0.0.1";    //������ַ
    unsigned int Port = 3306;   //���ݿ�˿ں�
    string User = "lzq";   //��½���ݿ��û���
    string PassWord = "0000";  //��½���ݿ�����
    string DBName = "kvlist"; //ʹ�����ݿ���
    //�������ݿ�
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }

    //ִ��sql��䣬�����ѯ�ɹ���mysql_query()�����᷵��0�����򣬷��ط���ֵ��ʾ��������
    mysql_query(con, "select count(*) from random group by `key`");

    MYSQL_RES *res;
    MYSQL_ROW row;
    //���ִ�н��
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    //��ȡ�ֶθ���������ѯ��õĽ�����м�������
    int nums = 0;  
    nums = mysql_num_fields(res);  //���ڱ��ṹ�Ļ�ȡ

    MYSQL_FIELD * fields;
    vector<int> volumeList;

    int tempL = 0;
    char *bkey = nullptr;
    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()������ָ���Ľ�����л�ȡһ�����ݷ��ظ�row�����������ʽ����row�ڲ����ַ�������ָ�루����ָ�룩
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
