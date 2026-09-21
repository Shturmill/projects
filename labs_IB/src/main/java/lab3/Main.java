package lab3;

import java.util.Collections;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import lab3.gui.LoginDialog;
import lab3.gui.MainFrame;
import lab3.gui.PassphraseDialog;

public class Main {

    public static void main(String[] args) {
        setupLookAndFeel();

        // расшифровываем учётные записи (при первом запуске - создаём)
        try {
            UserStore.createStorage();
            if (UserStore.exists()) {
                String passphrase = askPassphrase("Расшифрование базы данных",
                        "Пароль для расшифрования базы учётных записей:");
                if (passphrase == null) {
                    stop("Парольная фраза не введена. Программа будет закрыта.");
                }
                if (!UserStore.open(passphrase)) {
                    stop("Неверная парольная фраза. Программа будет закрыта.");
                }
            } else {
                String passphrase = askPassphrase("Создание базы данных",
                        "Парольная фраза для шифрования базы учётных записей:");
                if (passphrase == null) {
                    stop("Парольная фраза не задана. Программа будет закрыта.");
                }
                String again = askPassphrase("Создание базы данных", "Повторите парольную фразу:");
                if (!passphrase.equals(again)) {
                    stop("Парольные фразы не совпадают. Программа будет закрыта.");
                }
                UserStore.create(passphrase);
            }
        } catch (Exception e) {
            e.printStackTrace();
            stop("Ошибка при расшифровании учётных записей:\n" + e.getMessage());
        }

        // окно входа показывается до главного окна
        LoginDialog login = new LoginDialog(null, true);
        login.setVisible(true);

        if (login.getUser() == null) {
            exit();
        }

        Session.user = login.getUser();
        new MainFrame().setVisible(true);
    }

    // завершение работы: учётные записи снова шифруются, временная таблица удаляется
    public static void exit() {
        UserStore.close();
        Db.close();
        System.exit(0);
    }

    // окно запроса парольной фразы создаётся и уничтожается явно
    private static String askPassphrase(String title, String prompt) {
        PassphraseDialog dialog = new PassphraseDialog(null, true, title, prompt);
        dialog.setVisible(true);
        String passphrase = dialog.getPassphrase();
        dialog.dispose();
        return passphrase;
    }

    // сообщение и выход без сохранения
    private static void stop(String message) {
        JOptionPane.showMessageDialog(null, message, "Лабораторная работа №3", JOptionPane.WARNING_MESSAGE);
        Db.close();
        System.exit(0);
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
