package lab3.gui;

import lab3.Main;
import lab3.Session;

public class MainFrame extends javax.swing.JFrame {

    public MainFrame() {
        initComponents();
        setSize(getWidth(), 400);
        setLocationRelativeTo(null);

        lblStatus.setText("Пользователь: " + Session.user.getUsername()
                + " | Режим: " + (Session.isAdmin() ? "администратор" : "пользователь"));

        if (!Session.isAdmin()) {
            mnuUsers.setEnabled(false);
            btnUsers.setEnabled(false);
            btnAddUser.setEnabled(false);
        }
    }

    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jToolBar1 = new javax.swing.JToolBar();
        btnChangePassword = new javax.swing.JButton();
        btnUsers = new javax.swing.JButton();
        btnAddUser = new javax.swing.JButton();
        lblInfo = new javax.swing.JLabel();
        lblStatus = new javax.swing.JLabel();
        jMenuBar1 = new javax.swing.JMenuBar();
        mnuFile = new javax.swing.JMenu();
        mniChangePassword = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JPopupMenu.Separator();
        mniExit = new javax.swing.JMenuItem();
        mnuUsers = new javax.swing.JMenu();
        mniUsers = new javax.swing.JMenuItem();
        mniAddUser = new javax.swing.JMenuItem();
        mnuHelp = new javax.swing.JMenu();
        mniAbout = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("Лабораторная работа №3");
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                formWindowClosing(evt);
            }
        });

        jToolBar1.setRollover(true);

        btnChangePassword.setText("Сменить пароль");
        btnChangePassword.setFocusable(false);
        btnChangePassword.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        btnChangePassword.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        btnChangePassword.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnChangePasswordActionPerformed(evt);
            }
        });
        jToolBar1.add(btnChangePassword);

        btnUsers.setText("Список пользователей");
        btnUsers.setFocusable(false);
        btnUsers.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        btnUsers.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        btnUsers.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnUsersActionPerformed(evt);
            }
        });
        jToolBar1.add(btnUsers);

        btnAddUser.setText("Добавить пользователя");
        btnAddUser.setFocusable(false);
        btnAddUser.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        btnAddUser.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        btnAddUser.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnAddUserActionPerformed(evt);
            }
        });
        jToolBar1.add(btnAddUser);

        lblInfo.setText("Разграничение полномочий пользователей на основе парольной аутентификации");

        lblStatus.setText("Пользователь:");

        mnuFile.setText("Файл");

        mniChangePassword.setText("Сменить пароль");
        mniChangePassword.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mniChangePasswordActionPerformed(evt);
            }
        });
        mnuFile.add(mniChangePassword);
        mnuFile.add(jSeparator1);

        mniExit.setText("Выход");
        mniExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mniExitActionPerformed(evt);
            }
        });
        mnuFile.add(mniExit);

        jMenuBar1.add(mnuFile);

        mnuUsers.setText("Пользователи");

        mniUsers.setText("Список пользователей");
        mniUsers.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mniUsersActionPerformed(evt);
            }
        });
        mnuUsers.add(mniUsers);

        mniAddUser.setText("Добавить пользователя");
        mniAddUser.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mniAddUserActionPerformed(evt);
            }
        });
        mnuUsers.add(mniAddUser);

        jMenuBar1.add(mnuUsers);

        mnuHelp.setText("Справка");

        mniAbout.setText("О программе");
        mniAbout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mniAboutActionPerformed(evt);
            }
        });
        mnuHelp.add(mniAbout);

        jMenuBar1.add(mnuHelp);

        setJMenuBar(jMenuBar1);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jToolBar1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(lblInfo)
                    .addComponent(lblStatus))
                .addContainerGap(100, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jToolBar1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(lblInfo)
                .addGap(0, 0, Short.MAX_VALUE)
                .addComponent(lblStatus)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void mniChangePasswordActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mniChangePasswordActionPerformed
        changePassword();
    }//GEN-LAST:event_mniChangePasswordActionPerformed

    private void mniExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mniExitActionPerformed
        exit();
    }//GEN-LAST:event_mniExitActionPerformed

    private void mniUsersActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mniUsersActionPerformed
        showUsers();
    }//GEN-LAST:event_mniUsersActionPerformed

    private void mniAddUserActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mniAddUserActionPerformed
        addUser();
    }//GEN-LAST:event_mniAddUserActionPerformed

    private void mniAboutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mniAboutActionPerformed
        new AboutDialog(this, true).setVisible(true);
    }//GEN-LAST:event_mniAboutActionPerformed

    private void btnChangePasswordActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnChangePasswordActionPerformed
        changePassword();
    }//GEN-LAST:event_btnChangePasswordActionPerformed

    private void btnUsersActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnUsersActionPerformed
        showUsers();
    }//GEN-LAST:event_btnUsersActionPerformed

    private void btnAddUserActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnAddUserActionPerformed
        addUser();
    }//GEN-LAST:event_btnAddUserActionPerformed

    private void formWindowClosing(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_formWindowClosing
        exit();
    }//GEN-LAST:event_formWindowClosing

    private void changePassword() {
        new ChangePasswordDialog(this, true, Session.user, false).setVisible(true);
    }

    private void showUsers() {
        new UsersDialog(this, true).setVisible(true);
    }

    private void addUser() {
        new AddUserDialog(this, true).setVisible(true);
    }

    private void exit() {
        Main.exit();
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnAddUser;
    private javax.swing.JButton btnChangePassword;
    private javax.swing.JButton btnUsers;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JPopupMenu.Separator jSeparator1;
    private javax.swing.JToolBar jToolBar1;
    private javax.swing.JLabel lblInfo;
    private javax.swing.JLabel lblStatus;
    private javax.swing.JMenuItem mniAbout;
    private javax.swing.JMenuItem mniAddUser;
    private javax.swing.JMenuItem mniChangePassword;
    private javax.swing.JMenuItem mniExit;
    private javax.swing.JMenuItem mniUsers;
    private javax.swing.JMenu mnuFile;
    private javax.swing.JMenu mnuHelp;
    private javax.swing.JMenu mnuUsers;
    // End of variables declaration//GEN-END:variables
}
