# Окружение разработки для лабораторной работы №3.
#
#   nix-shell develop.nix     - войти в окружение (JDK, Maven, NetBeans)
#   netbeans                  - запустить IDE (File -> Open Project -> папка labs_IB),
#                               формы открываются на вкладке Design
#   mvn clean package         - собрать target/lab3.jar
#   java -jar target/lab3.jar - запустить программу
#
# Учётные записи хранятся в файлах рядом с программой:
#   users.txt      - зашифрованные учётные записи
#   users_temp.csv - расшифрованные, существует только во время работы программы
#
{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  name = "lab3-dev";

  packages = with pkgs; [
    jdk21          # NetBeans 30 требует Java 21+; проект компилируется под 17
    maven
    netbeans       # NetBeans с GUI Builder (файлы *.form)
  ];

  JAVA_HOME = "${pkgs.jdk21}";

  shellHook = ''
    echo "lab3: $(java -version 2>&1 | head -1)"
    echo "команды: netbeans | mvn clean package | java -jar target/lab3.jar"
  '';
}
