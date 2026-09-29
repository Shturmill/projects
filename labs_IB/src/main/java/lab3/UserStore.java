package lab3;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import javax.crypto.spec.SecretKeySpec;
import javax.swing.JOptionPane;

public class UserStore {

    private static final File FILE = new File("users.dat");
    private static final File TEMP = new File("users_temp.csv");
    private static final File IMPORT = new File("users_import.csv");

    private static SecretKeySpec key; // сеансовый ключ, null - файл ещё не открыт
    private static int imported; // сколько записей перенесено из выгрузки ЛР1

    public static File getTempFile() {
        return TEMP;
    }

    public static int getImported() {
        return imported;
    }

    // есть ли уже зашифрованный файл
    public static boolean exists() {
        return FILE.exists();
    }

    // первый запуск: учётные записи из выгрузки ЛР1 (если есть) или только ADMIN
    // с пустым паролем, файл сразу шифруется
    public static void create(String passphrase) throws Exception {
        key = Crypto.deriveKey(passphrase);
        writeTemp("");
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

    // расшифровать файл во временный, false - парольная фраза неверная
    // или файл изменён (нет учётной записи ADMIN, испорчены строки)
    public static boolean open(String passphrase) throws Exception {
        SecretKeySpec newKey = Crypto.deriveKey(passphrase);
        byte[] data = Crypto.decrypt(Files.readAllBytes(FILE.toPath()), newKey);
        String text = new String(data, StandardCharsets.UTF_8);

        if (!hasAdmin(text)) {
            return false;
        }

        key = newKey;
        writeTemp(text);
        importPlain();
        return true;
    }

    // зашифровать временный файл, старое содержимое users.dat затирается
    public static void save() {
        if (key == null) {
            return;
        }
        try {
            byte[] data = Crypto.encrypt(
                Files.readAllBytes(TEMP.toPath()),
                key
            );
            Files.write(FILE.toPath(), data);
        } catch (Exception e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(
                null,
                "Не удалось зашифровать учётные записи:\n" + e.getMessage(),
                "Ошибка",
                JOptionPane.ERROR_MESSAGE
            );
        }
    }

    // при выходе: зашифровать и удалить временный файл
    public static void close() {
        if (key == null) {
            return;
        }
        save();
        TEMP.delete();
        key = null;
    }

    // перенос выгрузки из ЛР1 (users_import.csv): записи с новыми именами добавляются,
    // данные шифруются, и только потом выгрузка удаляется
    private static void importPlain() throws Exception {
        if (!IMPORT.exists()) {
            return;
        }

        List<User> list = parse(
            new String(
                Files.readAllBytes(IMPORT.toPath()),
                StandardCharsets.UTF_8
            )
        );
        if (list == null) {
            JOptionPane.showMessageDialog(
                null,
                "Файл " +
                    IMPORT.getName() +
                    " имеет неверный формат, перенос не выполнен.",
                "Ошибка",
                JOptionPane.ERROR_MESSAGE
            );
            return;
        }

        UserDao dao = new UserDao();
        imported = 0;
        for (User user : list) {
            if (!dao.exists(user.getUsername())) {
                dao.insert(user);
                imported++;
            }
        }
        save();
        IMPORT.delete();
    }

    private static void writeTemp(String text) throws Exception {
        Files.write(TEMP.toPath(), text.getBytes(StandardCharsets.UTF_8));
    }

    // строки вида имя;пароль;блокировка;ограничения
    private static List<User> parse(String text) {
        List<User> users = new ArrayList<>();
        for (String line : text.split("\n")) {
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }
            String[] parts = line.split(";", -1);
            if (parts.length != 4) {
                return null;
            }
            User user = new User();
            user.setUsername(parts[0]);
            user.setPassword(parts[1]);
            user.setBlocked(Boolean.parseBoolean(parts[2]));
            user.setRestrictionsEnabled(Boolean.parseBoolean(parts[3]));
            users.add(user);
        }
        return users;
    }

    // правильность парольной фразы определяется по наличию учётной записи ADMIN
    private static boolean hasAdmin(String text) {
        List<User> users = parse(text);
        if (users == null) {
            return false;
        }
        for (User user : users) {
            if ("ADMIN".equals(user.getUsername())) {
                return true;
            }
        }
        return false;
    }
}
