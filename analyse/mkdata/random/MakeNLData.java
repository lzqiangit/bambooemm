package random;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.util.Random;

/**
 * 创建用于测试在不同键值对数量的数据下,最大容量l对查询时间的影响
 */
public class MakeNLData {

    public static void main(String[] args) throws Exception{
        
        Connection connection = DBHelper.getConnection();
        /********************* 删除之前的记录 ********************/
        String sql = "delete from random where 1 = 1";
        int executeUpdate = connection.createStatement().executeUpdate(sql);
        System.out.println("成功删除" + executeUpdate + "条记录");
        /********************* 随机生成记录 ********************/
        sql = "insert into random values (`key` ?, `value` ?);";
        PreparedStatement statement = null;
        
        statement = connection.prepareStatement(sql);

        /*=========================== 设置生成随机数的参数 ===========================*/
        sql = "insert into random values (?, ?, ?)";
        // int keyNum = 16384;
        int maxVolum = (int)Math.pow(2, 11); // 7 9 11 13
        int n = (int)Math.pow(2, 18);

        int num = 0;
        int key_index = 0;
        Random random = new Random();
        int valueNum = 0;
        String key = null;
        String value = null;
        int counter = 0;
        
        boolean firstFlag = true;

        statement = connection.prepareStatement(sql);
        while (num < n) {
            
            if (firstFlag) {
                 valueNum = maxVolum-1;
                 firstFlag = false;
            } else {
                do {
                    valueNum = random.nextInt(maxVolum);
                } while (valueNum == 0);
            }
            
            
            key = "key_" + key_index++;

            int curKeyLen = key.length();
            for (int i=curKeyLen; i<20; i++) {
                key = 'p' + key;
            }

            for (int j=0; j<=valueNum; j++) {
                value = Integer.toString(counter);
                //
                int curLen = value.length();
                for (int i=curLen; i<20; i++) {
                    value = 'p' + value;
                }

                statement.setString(1, key);    // 从1开始
                statement.setString(2, value);
                statement.setInt(3, j);

                statement.addBatch();
                if (counter % 1000 == 0) {
                    statement.executeBatch();
                    statement.clearBatch();
                }
                ++counter;
                if (++num == n) {
                    break;
                }
            }      
            
        }
        statement.executeBatch();
        statement.clearBatch();
        statement.close();
        connection.close();
        System.out.println("成功添加" + counter + "条记录！");
        System.out.println("n:" + n + " l:" + maxVolum);
        System.out.println("key_index:" + (key_index-1));
    }
}
