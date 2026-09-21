# Окружение разработки для лабораторной работы №1.
#
#   nix-shell develop.nix     - войти в окружение (JDK, Maven, NetBeans, PostgreSQL)
#   netbeans                  - запустить IDE (File -> Open Project -> папка lab1),
#                               формы открываются на вкладке Design
#   pg-start                  - поднять локальный PostgreSQL и создать базу lab1
#   pg-stop                   - остановить PostgreSQL
#   mvn clean package         - собрать target/lab1.jar
#
{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  name = "lab1-dev";

  packages = with pkgs; [
    jdk21          # NetBeans 30 требует Java 21+; проект компилируется под 17
    maven
    netbeans       # NetBeans с GUI Builder (файлы *.form)
    postgresql_16  # сервер и psql для локальной базы
  ];

  JAVA_HOME = "${pkgs.jdk21}";

  shellHook = ''
    # локальная база лежит в ./.pgdata, порт 5432, пользователь postgres без пароля
    export PGDATA="$PWD/.pgdata"
    export PGHOST=127.0.0.1
    export PGPORT=5432
    export PGUSER=postgres

    pg-start() {
      if [ ! -d "$PGDATA" ]; then
        initdb -U postgres --auth=trust -E UTF8 > /dev/null
      fi
      pg_ctl -D "$PGDATA" -l "$PGDATA/server.log" \
             -o "-p $PGPORT -h $PGHOST -k $PGDATA" start
      createdb lab1 2> /dev/null
      psql -d lab1 -f sql/init.sql
    }

    pg-stop() {
      pg_ctl -D "$PGDATA" stop -m fast
    }

    echo "lab1: $(java -version 2>&1 | head -1)"
    echo "команды: netbeans | pg-start | pg-stop | mvn clean package | java -jar target/lab1.jar"
  '';
}
