package random;
import java.sql.Connection;
import java.sql.DriverManager;
import java.util.Properties;

import javax.management.RuntimeErrorException;

/**
 * GetConnection
 */
public class DBHelper {

    private static final String url = "jdbc:mysql://localhost:3306/kvlist?rewriteBatchedStatements = true";
    private static final String user = "lzq";
    private static final String password = "0000";
    private static final String driver = "com.mysql.cj.jdbc.Driver";
    
    static {
        try {
            Class.forName(driver);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        

    }
    public static Connection getConnection() {
        Connection connection = null;
        try {
            connection = DriverManager.getConnection(url, user, password);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

        return connection;
    }
}