package lab3;

// Вариант 10: наличие латинских букв, символов кириллицы и цифр
public class PasswordValidator {

    public static final String DESCRIPTION =
            "Пароль должен содержать латинские буквы, символы кириллицы и цифры";

    // возвращает null, если пароль подходит, иначе текст ошибки
    public static String check(String password) {
        boolean hasLatin = false;
        boolean hasCyrillic = false;
        boolean hasDigit = false;

        for (char c : password.toCharArray()) {
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                hasLatin = true;
            } else if ((c >= 'А' && c <= 'я') || c == 'Ё' || c == 'ё') {
                hasCyrillic = true;
            } else if (Character.isDigit(c)) {
                hasDigit = true;
            }
        }

        if (hasLatin && hasCyrillic && hasDigit) {
            return null;
        }
        return DESCRIPTION;
    }
}
