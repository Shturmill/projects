# Lab 3 — Information security

**Using the cryptographic interface of the operating system to protect information**
Kanteev Sergei, IDB-23-08. Variant 10: RC4 stream cipher, MD5 hashing, no salt.

The program from lab 1 (password authentication with user roles, Java 17, Swing, forms built in the
NetBeans GUI Builder), extended with encryption of the account file. Instead of the Windows CryptoAPI
it uses the standard Java Cryptography Architecture: `MessageDigest("MD5")` turns the passphrase into
a 128-bit key and `Cipher("ARCFOUR")` encrypts the data.

## Data files

| File | Contents |
|---|---|
| `users.txt` | the accounts, encrypted and written as Base64 text, so the file opens and edits in any text editor |
| `users_temp.csv` | decrypted records `name;password;blocked;restrictions`, exists only while the program runs |
| `users_import.csv` | optional export from lab 1: new names are imported from it and the file is deleted |

All three are in `.gitignore` and never reach the repository.

## Build and run

```
nix-shell develop.nix        # JDK 21, Maven, NetBeans (java and mvn are not installed system-wide)
mvn clean package
java -jar target/lab3.jar
```

NetBeans: run `netbeans`, then File → Open Project → the `labs_IB` folder. Forms open on the Design tab.

## How it works

1. On the first run the program asks for a passphrase twice and creates `users.txt` holding a single
   `ADMIN` account with an empty password.
2. On later runs it asks for the passphrase, decrypts the data into the temporary file and looks for
   the `ADMIN` record. If it is missing, the passphrase is wrong: the program reports it and exits
   without touching the stored data.
3. On exit the data is encrypted again over the old content and the temporary file is deleted.
4. Login: an empty password leads straight to the main window. A password change is required only when
   the account has restrictions enabled and the current password does not satisfy them.

## Code

```
src/main/java/lab3/
├── Main.java               startup, passphrase, exit with re-encryption
├── Crypto.java             RC4 + MD5 (variant 10)
├── UserStore.java          file encryption and decryption into the temporary file
├── UserDao.java            reads and writes the temporary CSV
├── User.java, Session.java, PasswordValidator.java
└── gui/                    windows (*.java + *.form), PassphraseDialog asks for the passphrase
```
