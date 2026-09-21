package lab3;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

// Вариант 10: потоковый шифр RC4, хеширование MD5, без добавления к ключу случайного значения
public class Crypto {

    // сеансовый ключ из парольной фразы: хеш MD5 -> ключ RC4 (128 бит)
    public static SecretKeySpec deriveKey(String passphrase) throws Exception {
        MessageDigest md5 = MessageDigest.getInstance("MD5");
        byte[] hash = md5.digest(passphrase.getBytes(StandardCharsets.UTF_8));
        return new SecretKeySpec(hash, "ARCFOUR");
    }

    public static byte[] encrypt(byte[] data, SecretKeySpec key) throws Exception {
        Cipher cipher = Cipher.getInstance("ARCFOUR");
        cipher.init(Cipher.ENCRYPT_MODE, key);
        return cipher.doFinal(data);
    }

    public static byte[] decrypt(byte[] data, SecretKeySpec key) throws Exception {
        Cipher cipher = Cipher.getInstance("ARCFOUR");
        cipher.init(Cipher.DECRYPT_MODE, key);
        return cipher.doFinal(data);
    }
}
