#include "utils.hpp"
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include<algorithm>
using namespace std;

vector<KV *> LoadKVList() {
    MYSQL *con = NULL;
    con = mysql_init(con);//初始化
    if (con == NULL)
    {
        cout << "Init Connect ERROR" << endl;;
    }
    string url = "192.168.248.137";    //主机地址
    unsigned int Port = 3306;   //数据库端口号
    string User = "root";   //登陆数据库用户名
    string PassWord = "0000";  //登陆数据库密码
    string DBName = "kvlist"; //使用数据库名
    //链接数据库
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "Connect Database Error" << endl;
    }

    //执行sql语句，如果查询成功，mysql_query()函数会返回0；否则，返回非零值表示发生错误。
    mysql_query(con, "select * from KV_LIST_10_5");

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
    // fields = mysql_fetch_fields(res);  //属于表结构的获取
    // for(int i = 0; i < nums ; i++)
    // {
    //     cout<<fields[i].name<<"|";
    // }
    // cout<<endl;
    vector<KV *> kvList;
    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()函数从指定的结果集中获取一行数据返回给row，是数组的形式，即row内部是字符串数组指针（二级指针）
    {
        KV *kv = new KV(row[0], row[1], stoi(string(row[2])));
        //cout << kv->key << "|" << kv->value << "|" << kv->counter << endl;
        kvList.push_back(kv);
    }



    for(vector<KV *>::iterator it = kvList.begin(); it!=kvList.end(); it++){
        cout << (*it)->key << "|" << (*it)->value << "|" << (*it)->counter << "|"  << endl;
    }

    mysql_free_result(res);
    mysql_close(con);
    return kvList;
}