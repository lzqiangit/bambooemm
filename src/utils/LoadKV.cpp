#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <vector>
#include "bamboofilter/predefine.h"

using namespace std;


void LoadKV(vector<KV> kvList, int &size) {

    cout << "RUN SUCCESS!" << endl;
    MYSQL *con = NULL;
    con = mysql_init(con);//��ʼ��

    if (con == NULL)
    {
        cout << "��ʼ��ʧ��";
    }
    string url = "192.168.40.1";    //������ַ
    unsigned int Port = 3306;   //���ݿ�˿ں�
    string User = "root";   //��½���ݿ��û���
    string PassWord = "0000";  //��½���ݿ�����
    string DBName = "kvlist"; //ʹ�����ݿ���
    //�������ݿ�
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

    if (con == NULL)
    {
        cout << "�������ݿ�ʧ��";
    }
    //ִ��sql��䣬�����ѯ�ɹ���mysql_query()�����᷵��0�����򣬷��ط���ֵ��ʾ��������
    mysql_query(con, "select * from KV_LIST_10_5");

    MYSQL_RES *res;
    MYSQL_ROW row;
    //���ִ�н��
    res = mysql_use_result(con);
    const char * csname = "utf8";
    mysql_set_character_set(con, csname);

    //��ȡ�ֶθ���������ѯ��õĽ�����м�������
    int nums = 0;  
    nums = mysql_num_fields(res);  //���ڱ�ṹ�Ļ�ȡ
    cout << nums << endl;

    MYSQL_FIELD * fields;
    fields = mysql_fetch_fields(res);  //���ڱ�ṹ�Ļ�ȡ
    for(int i = 0; i < nums ; i++)
    {
        cout<<fields[i].name<<"|";
    }
    cout<<endl;

    while( (row = mysql_fetch_row(res)) != nullptr)  //mysql_fetch_row()������ָ���Ľ�����л�ȡһ�����ݷ��ظ�row�����������ʽ����row�ڲ����ַ�������ָ�루����ָ�룩
    {
        for(int i = 0; i < nums; i++)
        {
            cout<<row[i]<<"|" ;
        }
        cout << endl;
    }

    cout << "RUN OVER!";

    mysql_free_result(res);
    mysql_close(con);
}

