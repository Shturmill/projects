package lab3.gui;

import javax.swing.JOptionPane;
import lab3.PasswordValidator;
import lab3.User;
import lab3.UserDao;

public class ChangePasswordDialog extends javax.swing.JDialog {

    private UserDao dao = new UserDao();
    private User user;
    private boolean firstLogin;
    private boolean changed;

    public ChangePasswordDialog(java.awt.Frame parent, boolean modal, User user, boolean firstLogin) {
        super(parent, modal);
        this.user = user;
        this.firstLogin = firstLogin;
        initComponents();
        setLocationRelativeTo(null);
        getRootPane().setDefaultButton(btnOk);
        txtOld.setEchoChar('*');
        txtNew.setEchoChar('*');
        txtConfirm.setEchoChar('*');
        if (firstLogin) {
            setTitle(user.getPassword().isEmpty() ? "Установка пароля при первом входе" : "Пароль не соответствует ограничениям");
            txtOld.setEnabled(false);
        }
    }

    public boolean isChanged() {
        return changed;
    }

    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        txtOld = new javax.swing.JPasswordField();
        txtNew = new javax.swing.JPasswordField();
        txtConfirm = new javax.swing.JPasswordField();
        btnOk = new javax.swing.JButton();
        btnCancel = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Смена пароля");
        setResizable(false);

        jLabel1.setText("Старый пароль:");

        jLabel2.setText("Новый пароль:");

        jLabel3.setText("Подтверждение:");

        btnOk.setText("OK");
        btnOk.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnOkActionPerformed(evt);
            }
        });

        btnCancel.setText("Отмена");
        btnCancel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                btnCancelActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(jLabel1)
                            .addComponent(jLabel2)
                            .addComponent(jLabel3))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(txtOld, javax.swing.GroupLayout.DEFAULT_SIZE, 250, Short.MAX_VALUE)
                            .addComponent(txtNew, javax.swing.GroupLayout.DEFAULT_SIZE, 250, Short.MAX_VALUE)
                            .addComponent(txtConfirm, javax.swing.GroupLayout.DEFAULT_SIZE, 250, Short.MAX_VALUE)))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addComponent(btnOk)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(btnCancel)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel1)
                    .addComponent(txtOld, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(txtNew, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel3)
                    .addComponent(txtConfirm, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(btnOk)
                    .addComponent(btnCancel))
                .addContainerGap(20, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void btnOkActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnOkActionPerformed
        String oldPassword = new String(txtOld.getPassword());
        String newPassword = new String(txtNew.getPassword());
        String confirm = new String(txtConfirm.getPassword());

        if (!firstLogin && !user.getPassword().equals(oldPassword)) {
            JOptionPane.showMessageDialog(this, "Старый пароль введён неверно");
            return;
        }

        if (!newPassword.equals(confirm)) {
            JOptionPane.showMessageDialog(this, "Новый пароль и подтверждение не совпадают");
            return;
        }

        if (newPassword.isEmpty()) {
            JOptionPane.showMessageDialog(this, "Пароль не может быть пустым");
            return;
        }

        if (newPassword.contains(";")) {
            JOptionPane.showMessageDialog(this, "Пароль не должен содержать символ ;");
            return;
        }

        if (user.isRestrictionsEnabled()) {
            String error = PasswordValidator.check(newPassword);
            if (error != null) {
                JOptionPane.showMessageDialog(this, error);
                return;
            }
        }

        dao.updatePassword(user.getUsername(), newPassword);
        user.setPassword(newPassword);
        changed = true;
        JOptionPane.showMessageDialog(this, "Пароль изменён");
        dispose();
    }//GEN-LAST:event_btnOkActionPerformed

    private void btnCancelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_btnCancelActionPerformed
        dispose();
    }//GEN-LAST:event_btnCancelActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnCancel;
    private javax.swing.JButton btnOk;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JPasswordField txtConfirm;
    private javax.swing.JPasswordField txtNew;
    private javax.swing.JPasswordField txtOld;
    // End of variables declaration//GEN-END:variables
}
