#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>

using namespace std;

int main(int argc, char const *argv[])
{
    unordered_map<string, int *> *emmST = new unordered_map<string, int *>();
    char *key = new char[6];
    memset(key, 0, 6);
    sprintf(key, "%s", "hello");

    if (emmST->find(key) == emmST->end())
    {
        (*emmST)[key] = new int[2]{6, 7};
    }

    string keyStr = "hello";
    cout << key << "\t" << (*emmST)[key][0] << "\t" << (*emmST)[keyStr][1] << endl;

    (*emmST)[keyStr][1]++;

    cout << key << "\t" << (*emmST)[key][0] << "\t" << (*emmST)[keyStr][1] << endl;
    return 0;
}
