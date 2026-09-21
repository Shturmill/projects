package lab3;

// Кто сейчас работает с программой
public class Session {

    public static User user;

    // администратор определяется по имени ADMIN
    public static boolean isAdmin() {
        return user != null && "ADMIN".equals(user.getUsername());
    }
}
