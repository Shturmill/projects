package lab1;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JOptionPane;

// Все запросы к таблице users
public class UserDao {

    // найти пользователя по имени, null - если такого нет
    public User findByName(String name) {
        String sql = "SELECT id, username, password, blocked, restrictions_enabled FROM users WHERE username = ?";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ps.setString(1, name);
            ResultSet rs = ps.executeQuery();
            if (rs.next()) {
                return read(rs);
            }
        } catch (SQLException e) {
            error(e);
        }
        return null;
    }

    // все пользователи, ADMIN первым
    public List<User> findAll() {
        List<User> list = new ArrayList<>();
        String sql = "SELECT id, username, password, blocked, restrictions_enabled FROM users ORDER BY id";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ResultSet rs = ps.executeQuery();
            while (rs.next()) {
                list.add(read(rs));
            }
        } catch (SQLException e) {
            error(e);
        }
        return list;
    }

    // добавить нового пользователя
    public void insert(User user) {
        String sql = "INSERT INTO users (username, password, blocked, restrictions_enabled) VALUES (?, ?, ?, ?)";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ps.setString(1, user.getUsername());
            ps.setString(2, user.getPassword());
            ps.setBoolean(3, user.isBlocked());
            ps.setBoolean(4, user.isRestrictionsEnabled());
            ps.executeUpdate();
        } catch (SQLException e) {
            error(e);
        }
    }

    // сменить пароль
    public void updatePassword(String name, String newPassword) {
        String sql = "UPDATE users SET password = ? WHERE username = ?";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ps.setString(1, newPassword);
            ps.setString(2, name);
            ps.executeUpdate();
        } catch (SQLException e) {
            error(e);
        }
    }

    // блокировка и ограничения на пароль
    public void updateFlags(User user) {
        String sql = "UPDATE users SET blocked = ?, restrictions_enabled = ? WHERE username = ?";
        try (PreparedStatement ps = Db.get().prepareStatement(sql)) {
            ps.setBoolean(1, user.isBlocked());
            ps.setBoolean(2, user.isRestrictionsEnabled());
            ps.setString(3, user.getUsername());
            ps.executeUpdate();
        } catch (SQLException e) {
            error(e);
        }
    }

    // есть ли уже такое имя
    public boolean exists(String name) {
        return findByName(name) != null;
    }

    // прочитать одну строку результата
    private User read(ResultSet rs) throws SQLException {
        User user = new User();
        user.setId(rs.getInt("id"));
        user.setUsername(rs.getString("username"));
        user.setPassword(rs.getString("password"));
        user.setBlocked(rs.getBoolean("blocked"));
        user.setRestrictionsEnabled(rs.getBoolean("restrictions_enabled"));
        return user;
    }

    // сообщение об ошибке БД
    private void error(SQLException e) {
        e.printStackTrace();
        JOptionPane.showMessageDialog(null,
                "Ошибка базы данных:\n" + e.getMessage(),
                "Ошибка", JOptionPane.ERROR_MESSAGE);
    }
}
