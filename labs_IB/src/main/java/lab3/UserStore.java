package lab3;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;
import javax.crypto.spec.SecretKeySpec;
import javax.swing.JOptionPane;

// Учётные записи хранятся в БД только в зашифрованном виде (таблица users_encrypted).
// На время работы они расшифровываются во временную таблицу users,
// которую PostgreSQL удаляет сам при закрытии соединения.
public class UserStore {

    private static SecretKeySpec key;   // сеансовый ключ, null - база ещё не открыта
    private static int imported;        // сколько записей перенесено из открытой таблицы ЛР1

    public static int getImported() {
        return imported;
    }

    // таблица с зашифрованными учётными записями (аналог файла)
    public static void createStorage() throws SQLException {
        try (Statement st = Db.get().createStatement()) {
            st.executeUpdate("CREATE TABLE IF NOT EXISTS users_encrypted ("
                    + "id INTEGER PRIMARY KEY, "
                    + "data BYTEA NOT NULL)");
        }
    }

    // есть ли уже зашифрованные учётные записи
    public static boolean exists() throws SQLException {
        return readEncrypted() != null;
    }

    // первый запуск: учётные записи из ЛР1 (если есть) или только ADMIN с пустым паролем,
    // база сразу шифруется
    public static void create(String passphrase) throws Exception {
        key = Crypto.deriveKey(passphrase);
        createTempTable();
        importPlain();
        UserDao dao = new UserDao();
        if (!dao.exists("ADMIN")) {
            User admin = new User();
            admin.setUsername("ADMIN");
            admin.setPassword("");
            dao.insert(admin);
        }
        save();
    }

    // расшифровать учётные записи во временную таблицу,
    // false - парольная фраза неверная (нет учётной записи ADMIN)
    public static boolean open(String passphrase) throws Exception {
        SecretKeySpec newKey = Crypto.deriveKey(passphrase);
        List<User> users = parse(Crypto.decrypt(readEncrypted(), newKey));
        if (users == null || !hasAdmin(users)) {
            return false;
        }

        key = newKey;
        createTempTable();
        UserDao dao = new UserDao();
        for (User user : users) {
            dao.insert(user);
        }
        importPlain();
        return true;
    }

    // перенос открытой таблицы users из ЛР1 (public.users): записи с новыми именами
    // добавляются, данные шифруются, и только потом открытая таблица удаляется
    private static void importPlain() throws SQLException {
        try (Statement st = Db.get().createStatement()) {
            ResultSet rs = st.executeQuery("SELECT to_regclass('public.users') IS NOT NULL");
            rs.next();
            if (!rs.getBoolean(1)) {
                return;
            }
            imported = st.executeUpdate("INSERT INTO pg_temp.users "
                    + "(username, password, blocked, restrictions_enabled) "
                    + "SELECT username, password, blocked, restrictions_enabled FROM public.users "
                    + "WHERE username NOT IN (SELECT username FROM pg_temp.users) ORDER BY id");
            save();
            st.executeUpdate("DROP TABLE public.users");
        }
    }

    // зашифровать текущие учётные записи, старое содержимое затирается
    public static void save() {
        if (key == null) {
            return;
        }
        String sql = "INSERT INTO users_encrypted (id, data) VALUES (1, ?) "
                + "ON CONFLICT (id) DO UPDATE SET data = EXCLUDED.data";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ps.setBytes(1, Crypto.encrypt(serialize(new UserDao().findAll()), key));
            ps.executeUpdate();
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(null,
                    "Не удалось зашифровать учётные записи:\n" + e.getMessage(),
                    "Ошибка", JOptionPane.ERROR_MESSAGE);
        }
    }

    // при выходе: зашифровать и удалить временную таблицу
    public static void close() {
        if (key == null) {
            return;
        }
        save();
        try (Statement st = Db.get().createStatement()) {
            st.executeUpdate("DROP TABLE IF EXISTS pg_temp.users");
        } catch (SQLException e) {
            e.printStackTrace();
        }
        key = null;
    }

    private static void createTempTable() throws SQLException {
        try (Statement st = Db.get().createStatement()) {
            st.executeUpdate("CREATE TEMPORARY TABLE users ("
                    + "id SERIAL PRIMARY KEY, "
                    + "username VARCHAR(50) NOT NULL UNIQUE, "
                    + "password VARCHAR(100) NOT NULL DEFAULT '', "
                    + "blocked BOOLEAN NOT NULL DEFAULT FALSE, "
                    + "restrictions_enabled BOOLEAN NOT NULL DEFAULT FALSE)");
        }
    }

    private static byte[] readEncrypted() throws SQLException {
        try (Statement st = Db.get().createStatement()) {
            ResultSet rs = st.executeQuery("SELECT data FROM users_encrypted WHERE id = 1");
            if (rs.next()) {
                return rs.getBytes("data");
            }
        }
        return null;
    }

    // записи: количество, затем имя, пароль, блокировка, ограничения
    private static byte[] serialize(List<User> users) throws IOException {
        ByteArrayOutputStream bytes = new ByteArrayOutputStream();
        DataOutputStream out = new DataOutputStream(bytes);
        out.writeInt(users.size());
        for (User user : users) {
            out.writeUTF(user.getUsername());
            out.writeUTF(user.getPassword());
            out.writeBoolean(user.isBlocked());
            out.writeBoolean(user.isRestrictionsEnabled());
        }
        out.flush();
        return bytes.toByteArray();
    }

    // null - данные не читаются (расшифрованы неверным ключом)
    private static List<User> parse(byte[] data) {
        try {
            DataInputStream in = new DataInputStream(new ByteArrayInputStream(data));
            int count = in.readInt();
            if (count < 0 || count > data.length) {
                return null;
            }
            List<User> users = new ArrayList<>();
            for (int i = 0; i < count; i++) {
                User user = new User();
                user.setUsername(in.readUTF());
                user.setPassword(in.readUTF());
                user.setBlocked(in.readBoolean());
                user.setRestrictionsEnabled(in.readBoolean());
                users.add(user);
            }
            return users;
        } catch (IOException e) {
            return null;
        }
    }

    private static boolean hasAdmin(List<User> users) {
        for (User user : users) {
            if ("ADMIN".equals(user.getUsername())) {
                return true;
            }
        }
        return false;
    }
}
