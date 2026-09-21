package lab1;

import java.sql.SQLException;
import java.sql.Statement;
import java.util.Collections;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import lab1.gui.LoginDialog;
import lab1.gui.MainFrame;

public class Main {

    public static void main(String[] args) {
        setupLookAndFeel();

        // при первом запуске создаём таблицу и учётную запись ADMIN
        try (Statement st = Db.get().createStatement()) {
            st.executeUpdate("CREATE TABLE IF NOT EXISTS users ("
                    + "id SERIAL PRIMARY KEY, "
                    + "username VARCHAR(50) NOT NULL UNIQUE, "
                    + "password VARCHAR(100) NOT NULL DEFAULT '', "
                    + "blocked BOOLEAN NOT NULL DEFAULT FALSE, "
                    + "restrictions_enabled BOOLEAN NOT NULL DEFAULT FALSE)");
        } catch (SQLException e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(null,
                    "Ошибка базы данных:\n" + e.getMessage(),
                    "Ошибка", JOptionPane.ERROR_MESSAGE);
            System.exit(1);
        }

        UserDao dao = new UserDao();
        if (!dao.exists("ADMIN")) {
            User admin = new User();
            admin.setUsername("ADMIN");
            admin.setPassword("");
            dao.insert(admin);
        }

        // окно входа показывается до главного окна
        LoginDialog login = new LoginDialog(null, true);
        login.setVisible(true);

        if (login.getUser() == null) {
            Db.close();
            System.exit(0);
        }

        Session.user = login.getUser();
        new MainFrame().setVisible(true);
    }

    // стандартный вид Swing (Metal) и шрифт 20 кегля во всех окнах
    private static void setupLookAndFeel() {
        try {
            UIManager.put("swing.boldMetal", Boolean.FALSE);
            UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }

        for (Object key : Collections.list(UIManager.getDefaults().keys())) {
            Object value = UIManager.get(key);
            if (value instanceof FontUIResource) {
                FontUIResource font = (FontUIResource) value;
                UIManager.put(key, new FontUIResource(font.getFamily(), font.getStyle(), 20));
            }
        }
    }
}
