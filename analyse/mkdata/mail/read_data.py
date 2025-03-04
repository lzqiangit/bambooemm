import os
import re
import random
import pickle
import mysql.connector

file_amount = 20000
kv_amount = 2 ** 22



stopWords = {'ourselves', 'hers', 'between', 'yourself', 'but', 'again', 'there', 'about', 'once', 'during', 'out',
             'very', 'having', 'with', 'they', 'own', 'an', 'be', 'some', 'for', 'do', 'its', 'yours', 'such',
             'into',
             'of', 'most', 'itself', 'other', 'off', 'is', 's', 'am', 'or', 'who', 'as', 'from', 'him', 'each',
             'the',
             'themselves', 'until', 'below', 'are', 'we', 'these', 'your', 'his', 'through', 'don', 'nor', 'me',
             'were', 'her', 'more', 'himself', 'this', 'down', 'should', 'our', 'their', 'while', 'above', 'both',
             'up', 'to', 'ours', 'had', 'she', 'all', 'no', 'when', 'at', 'any', 'before', 'them', 'same', 'and',
             'been', 'have', 'in', 'will', 'on', 'does', 'yourselves', 'then', 'that', 'because', 'what', 'over',
             'why', 'so', 'can', 'did', 'not', 'now', 'under', 'he', 'you', 'herself', 'has', 'just', 'where',
             'too',
             'only', 'myself', 'which', 'those', 'i', 'after', 'few', 'whom', 't', 'being', 'if', 'theirs', 'my',
             'against', 'a', 'by', 'doing', 'it', 'how', 'further', 'was', 'here', 'than'}


def dir_filter(file):
    return file.startswith(".")


def file_filter(file):
    return not file.endswith(".")


# 深度优先搜索
def files_from_dir(path, target_amount):
    files = []
    def dfs_dir(target_path):
        if len(files) == target_amount:
            return
        if os.path.isdir(target_path) and not dir_filter(target_path):
            subs = os.listdir(target_path)
            for sub in subs:
                dfs_dir(os.path.join(target_path, sub))
                if len(files) == target_amount:
                    break
        elif os.path.isfile(target_path) and not file_filter(target_path):
            files.append(target_path)
        else:
            print("skip " + target_path)

    dfs_dir(path)
    return files


# 解析文件 获取文件中的所有关键字（剔除文件中的标点符号和非关键字）
def file_parser(file):
    with open(file, 'r') as f:
        c = []
        try:
            lines = f.readlines()
            # print(lines)
            reach_head = False
            seen = set()
            for line in lines:
                if line.startswith('X-FileName'):
                    reach_head = True
                    continue
                # skip mail header
                if not reach_head:
                    continue
                # skip mail forward and appended mail
                if 'Forwarded by' in line:
                    continue
                if 'Original Message' in line:
                    continue
                if 'From:' in line:
                    continue
                if 'To:' in line:
                    continue
                if 'Cc:' in line:
                    continue
                if 'Sent:' in line:
                    continue
                if 'Subject:' in line:
                    continue
                if 'cc:' in line:
                    continue
                if 'subject:' in line:
                    continue
                if 'Subject:' in line:
                    continue
                if 'from:' in line:
                    continue
                # line = line.replace('\n',' ')
                line = re.sub(r"[^\s]*@[^\s]*", " ", line)
                line = re.sub(r"[^A-Za-z]", " ", line).lower()
                # print(line.split())

                # remove duplicate words
                tmp = line.split()
                line = []
                for l in tmp:
                    if len(l) >= 2 and l not in stopWords and l not in seen:
                        seen.add(l)
                        line.append(l)
                line = ' '.join(line)
                if len(line) != 0:
                    c.append(line)
                    # print(line)
        except UnicodeDecodeError:
            print(file)
            return ''
        # c_lists = content.split()
        # content = ' '.join([c for c in c_lists if enDict.check(c)]) # check if is a word
    return ' '.join(c)


# 解析文件
def get_corpus_from_dir(path, target_amount):
    fs = files_from_dir(path, target_amount + target_amount // 50)
    print("geting corpus from files")
    cps = []
    skip_count = 0
    for i in range(len(fs)):
        content = file_parser(fs[i])
        # print("content")
        # print(content)
        if len(content) == 0:
            skip_count += 1
            continue
        cps.append(content)
    # print("skip:", skip_count)
    # print("total:", len(cps))
    return cps[:target_amount]

# 获得关键字到文件的列表
def build_key_to_file_list(bv):
    Kw_File = dict()
    cur_kv_amount = 0
    for i, content_str in enumerate(bv):
        content_list = content_str.split()
        cur_kv_amount += len(content_list)
        for content in content_list:
            if len(content)>14:
                cur_kv_amount -= 1
                continue
            if content not in Kw_File:
                Kw_File[content] = [str(i).zfill(16)]
            else:
                Kw_File[content].append(str(i).zfill(16))
        if cur_kv_amount >= kv_amount:
            break
    print("FINAL FILE INDEX:" + str(i))
      
    print("INSERT KV NUM:" + str(cur_kv_amount))
    #######################为每个kw生成一个nonce
    # kw_nonce={}
    # for kw in Kw_File:
    #     st=os.urandom(16)    #生成16位byte
    #     kw_nonce[kw]=st
    #     Kw_File[kw].insert(0,kw_nonce[kw])

    ####################真正使用的
    # Kw_File_Use={}
    # for kw in Kw_File:
    #     if len(kw)<14:
    #         Kw_File_Use[kw]=Kw_File[kw]
    return Kw_File

# 保存到数据库
def save_to_db(kvs):
    # 建立连接
    conn = mysql.connector.connect(
        host="localhost",
        user="lzq",
        password="0000",
        database="kvlist"
    )

    # 创建游标对象
    cursor = conn.cursor()

    # 删除所有数据
    cursor.execute("delete from random where 1 = 1")
    results = cursor.fetchall()
    print(results)
    
    # 遍历字典，插入新的数据
    sql = "INSERT INTO random VALUES (%s, %s, %s)"
    data = []
    
    
    for kw in Kw_File_Use:
        # 将kw填充至20字节
        pad_len = 20 - len(kw)
        pad_kw = '*' * pad_len + kw
        counter = 0
        for j in Kw_File_Use[kw]:
            pad_len = 20 - len(j)
            pad_j = '0' * pad_len + j
            data.append((pad_kw, pad_j, counter))
            counter += 1
            if len(data) >= 50000:
                cursor.executemany(sql, data)
                data = []
                # conn.commit()

    # 批量插入数据
    cursor.executemany(sql, data)
    data = []
    # 提交事务 
    conn.commit()

    # 关闭游标和连接
    cursor.close()
    conn.close()

addchennonce=st=os.urandom(16)    #生成16位byte
addfileID='0000000000000000'

addchennonce1=st=os.urandom(16)    #生成16位byte
addfileID1='0000000000000000'

#调用函数
bv = get_corpus_from_dir("/home/lzq/data/maildir", file_amount)
Kw_File_Use=build_key_to_file_list(bv)

# Kw_File_Use['chen']=[addchennonce,addfileID]
# Kw_File_Use['zhang']=[addchennonce1,addfileID1]
# print(Kw_File_Use)




# Kw_File_Use['chen']=[1]

sum=0
word=0
for kw in Kw_File_Use:
    word=word+1
    for j in Kw_File_Use[kw]:
        sum=sum+1




print("sum:{}, word:{}".format(sum, word))

save_to_db(Kw_File_Use)

f_Kw_File_Use=open('/home/lzq/data/Kw_File_Use.txt','wb')
pickle.dump(Kw_File_Use, f_Kw_File_Use, 0)
f_Kw_File_Use.close()
# print(len(Kw_File_Use['john']))

print("word number: " + str(word))
# print(sum)

# a='zhang' in Kw_File_Use.keys()
# print(a)


###########
#49个文件-------9899～10000 pair---10K     0.711297s

#116个文件------19906~20000 pair---20k     1.335329

#198个文件------39528~40000 pair---40k     2.512464

#388个文件------79975～80000 pair---80k     5.071838

#876个文件------159966~160000 pair---160k    10.224763s


