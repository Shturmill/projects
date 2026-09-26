package lab3;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JOptionPane;

// Работа с временным файлом учётных записей
// (строки вида имя;пароль;блокировка;ограничения)
public class UserDao {

    // найти пользователя по имени, null - если такого нет
    public User findByName(String name) {
        for (User user : findAll()) {
            if (user.getUsername().equals(name)) {
                return user;
            }
        }
        return null;
    }

    // все пользователи, ADMIN первым
    public List<User> findAll() {
        List<User> users = new ArrayList<>();
        try {
            for (String line : Files.readAllLines(UserStore.getTempFile().toPath(), StandardCharsets.UTF_8)) {
                if (line.trim().isEmpty()) {
                    continue;
                }
                String[] parts = line.split(";", -1);
                User user = new User();
                user.setUsername(parts[0]);
                user.setPassword(parts[1]);
                user.setBlocked(Boolean.parseBoolean(parts[2]));
                user.setRestrictionsEnabled(Boolean.parseBoolean(parts[3]));
                users.add(user);
            }
        } catch (IOException e) {
            error(e);
        }
        return users;
    }

    // добавить нового пользователя
    public void insert(User user) {
        List<User> users = findAll();
        users.add(user);
        writeAll(users);
    }

    // сменить пароль
    public void updatePassword(String name, String newPassword) {
        List<User> users = findAll();
        for (User user : users) {
            if (user.getUsername().equals(name)) {
                user.setPassword(newPassword);
            }
        }
        writeAll(users);
    }

    // блокировка и ограничения на пароль
    public void updateFlags(User user) {
        List<User> users = findAll();
        for (User u : users) {
            if (u.getUsername().equals(user.getUsername())) {
                u.setBlocked(user.isBlocked());
                u.setRestrictionsEnabled(user.isRestrictionsEnabled());
            }
        }
        writeAll(users);
    }

    // есть ли уже такое имя
    public boolean exists(String name) {
        return findByName(name) != null;
    }

    // записать всех пользователей во временный файл
    private void writeAll(List<User> users) {
        StringBuilder text = new StringBuilder();
        for (User user : users) {
            text.append(user.getUsername()).append(';')
                    .append(user.getPassword()).append(';')
                    .append(user.isBlocked()).append(';')
                    .append(user.isRestrictionsEnabled()).append('\n');
        }
        try {
            Files.write(UserStore.getTempFile().toPath(), text.toString().getBytes(StandardCharsets.UTF_8));
        } catch (IOException e) {
            error(e);
        }
    }

    // сообщение об ошибке работы с файлом
    private void error(IOException e) {
        e.printStackTrace();
        JOptionPane.showMessageDialog(null,
                "Ошибка при работе с файлом учётных записей:\n" + e.getMessage(),
                "Ошибка", JOptionPane.ERROR_MESSAGE);
    }
}
