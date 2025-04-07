#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QKeySequence>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // Устанавливаем начальный заголовок окна
        setWindowTitle("Приложение с меню");

        // Создаём главное меню
        QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));
        QMenu *editMenu = menuBar()->addMenu(tr("Правка"));
        QMenu *helpMenu = menuBar()->addMenu(tr("Справка"));

        // Создаём действия для подменю "Файл"
        QAction *openAction = new QAction(tr("Открыть"), this);
        openAction->setShortcut(QKeySequence::Open); // Ctrl+O
        fileMenu->addAction(openAction);

        QAction *saveAction = new QAction(tr("Сохранить"), this);
        saveAction->setShortcut(QKeySequence::Save); // Ctrl+S
        fileMenu->addAction(saveAction);

        QAction *exitAction = new QAction(tr("Выход"), this);
        exitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4)); // Ctrl+F4
        fileMenu->addAction(exitAction);

        // Подключаем действия к слотам
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpen);
        connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
        connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    }

private slots:
    void onOpen() {
        QMessageBox::information(this, tr("Открыть"), tr("Вы выбрали пункт 'Открыть'."));
    }

    void onSave() {
        QMessageBox::information(this, tr("Сохранить"), tr("Вы выбрали пункт 'Сохранить'."));
    }

    void onExit() {
        QMessageBox::information(this, tr("Выход"), tr("Вы выбрали пункт 'Выход'."));
        close(); // Закрываем окно после сообщения
    }
};

#include "main.moc" // Необходим для обработки moc-файлов

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}