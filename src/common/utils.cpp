#include "utils.hpp"
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <algorithm>
#include <string.h>
using namespace std;

vector<KV *> LoadKVList() {
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
    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()函数从指定的结果集中获取一行数据返回给row，是数组的形式，即row内部是字符串数组指针（二级指针）
    {
        char *key = new char[(strlen(row[0]) + 1)];
        char *value = new char[(strlen(row[1]) + 1)];
        strcpy(key, row[0]);
        strcpy(value, row[1]);
        KV *kv = new KV(key, value, stoi(string(row[2])));
        kvList.push_back(kv);
    }
    mysql_free_result(res);
    mysql_close(con);
    return kvList;
}