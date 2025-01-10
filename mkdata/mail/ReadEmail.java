package mail;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import javax.management.RuntimeErrorException;

public class ReadEmail {

    public static void main(String[] args) {
        ArrayList<String> mailPaths = getEmailPath("/home/lzq/data/maildir");
        
    }


    /**
     * 解析邮件当中的关键字
     * @param mailPath
     * @return
     */
    private static ArrayList<String> ResolveEmail(String mailPath) {
        

        try {
            ArrayList<String> data = new ArrayList<String>();
            BufferedReader reader = new BufferedReader(new FileReader(mailPath));
            String line = null;
            while ( (line = reader.readLine()) != null ) {
                line.trim();
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
       
    }
    /**
     * 获取邮件路径
     * @param root  
     * @return
     */
    private static ArrayList<String> getEmailPath(String root) {
        ArrayList<String> mailPaths = new ArrayList<String>();
        File rootFold = new File(root);
        if (rootFold.isDirectory()) {
            File[] files = rootFold.listFiles();
            for (File file : files) {
                if (file.isDirectory()) {
                    mailPaths.addAll(getEmailPath(file.getAbsolutePath()));
                } else {
                    mailPaths.add(file.getAbsolutePath());
                }
            }
            return mailPaths;
        } else {
            System.out.println("错误的路径");
            return mailPaths;
        }
        
    }
    
}
