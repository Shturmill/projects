package lab3;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.util.Properties;
import javax.swing.JOptionPane;

// Подключение к PostgreSQL
public class Db {

    private static Connection connection;

    // одно соединение на всю программу
    public static Connection get() {
        if (connection == null) {
            try {
                Properties props = new Properties();
                InputStream in = Db.class.getResourceAsStream("/db.properties");
                props.load(new InputStreamReader(in, "UTF-8"));
                in.close();
                connection = DriverManager.getConnection(
                        props.getProperty("db.url"),
                        props.getProperty("db.user"),
                        props.getProperty("db.password"));
            } catch (Exception e) {
                e.printStackTrace();
                JOptionPane.showMessageDialog(null,
                        "Не удалось подключиться к базе данных:\n" + e.getMessage(),
                        "Ошибка", JOptionPane.ERROR_MESSAGE);
                System.exit(1);
            }
        }
        return connection;
    }

    // закрыть соединение при выходе
    public static void close() {
        try {
            if (connection != null) {
                connection.close();
                connection = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
