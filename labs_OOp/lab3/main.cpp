#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QKeySequence>
#include <QStatusBar>
#include <QThread>
#include <QProgressBar>
#include <QTimer>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <omp.h>

// Класс PiThread для вычисления числа π в фоновом потоке с использованием OpenMP
class PiThread : public QThread {
    Q_OBJECT
public:
    PiThread(int iterations, QObject *parent = nullptr)
        : QThread(parent), iterations(iterations), completed_iterations(0) {}

    // Атомарный счётчик завершённых итераций
    std::atomic<int> completed_iterations;

signals:
    // Сигнал для передачи результата вычислений
    void resultReady(const QString &result);

protected:
    void run() override {
        double sum = 0.0;
        int total_iterations = iterations;

        // Параллельное вычисление суммы с использованием OpenMP
        #pragma omp parallel for reduction(+:sum)
        for (int k = 0; k < total_iterations; ++k) {
            double term = (1.0 / pow(16, k)) *
                          (4.0 / (8 * k + 1) - 2.0 / (8 * k + 4) -
                           1.0 / (8 * k + 5) - 1.0 / (8 * k + 6));
            sum += term;
            completed_iterations.fetch_add(1, std::memory_order_relaxed);
        }

        // Форматирование результата с высокой точностью
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(100) << "value of π: " << sum;
        emit resultReady(QString::fromStdString(oss.str()));
    }

private:
    int iterations; // Количество итераций для вычисления
};

// Класс MainWindow для основного окна приложения
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Приложение");

        // Создание меню
        QMenu *fileMenu = menuBar()->addMenu(tr("Файл"));
        QMenu *editMenu = menuBar()->addMenu(tr("Правка"));
        QMenu *helpMenu = menuBar()->addMenu(tr("Справка"));

        QAction *openAction = new QAction(tr("Открыть"), this);
        openAction->setShortcut(QKeySequence::Open);
        fileMenu->addAction(openAction);

        QAction *saveAction = new QAction(tr("Сохранить"), this);
        saveAction->setShortcut(QKeySequence::Save);
        fileMenu->addAction(saveAction);

        QAction *exitAction = new QAction(tr("Выход"), this);
        exitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
        fileMenu->addAction(exitAction);

        // Подключение действий меню
        connect(openAction, &QAction::triggered, this, &MainWindow::onOpen);
        connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
        connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

        // Настройка центрального виджета и布局
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        button = new QPushButton(tr("Нажми меня"), this);
        layout->addWidget(button, 0, Qt::AlignCenter);

        input = new QLineEdit(this);
        layout->addWidget(input, 0, Qt::AlignCenter);

        startButton = new QPushButton(tr("Старт"), this);
        layout->addWidget(startButton, 0, Qt::AlignCenter);

        progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->setAlignment(Qt::AlignCenter);
        layout->addWidget(progressBar, 0, Qt::AlignCenter);

        layout->addStretch();

        statusBar = new QStatusBar(this);
        setStatusBar(statusBar);

        // Подключение кнопок
        connect(button, &QPushButton::clicked, this, &MainWindow::updateWindowTitle);
        connect(startButton, &QPushButton::clicked, this, &MainWindow::startPiCalculation);
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
        close();
    }

    void updateWindowTitle() {
        QString text = input->text();
        if (!text.isEmpty()) {
            setWindowTitle(text);
        }
    }

    void startPiCalculation() {
        bool ok;
        int iterations = input->text().toInt(&ok);
        if (!ok || iterations <= 0) {
            statusBar->showMessage(tr("Ошибка: введите положительное число итераций"));
            return;
        }

        startButton->setEnabled(false);
        progressBar->setValue(0);
        progressBar->setStyleSheet("");

        PiThread *thread = new PiThread(iterations, this);
        connect(thread, &PiThread::resultReady, this, &MainWindow::handlePiResult);
        connect(thread, &PiThread::finished, thread, &QObject::deleteLater);

        // Завершение вычислений: установка 100% и зелёного цвета
        connect(thread, &PiThread::finished, this, [this]() {
            progressBar->setValue(100);
            progressBar->setStyleSheet("QProgressBar::chunk { background-color: green; }");
            startButton->setEnabled(true);
        });

        // Таймер для обновления прогресс-бара
        QTimer *timer = new QTimer(this);
        timer->setInterval(100);
        connect(timer, &QTimer::timeout, this, [this, thread, iterations]() {
            int completed = thread->completed_iterations.load(std::memory_order_relaxed);
            int progress = static_cast<int>((completed * 100.0) / iterations);
            if (progress < 100) {
                progressBar->setValue(progress);
            }
        });
        timer->start();

        connect(thread, &PiThread::finished, timer, &QTimer::stop);
        connect(thread, &PiThread::finished, timer, &QObject::deleteLater);

        thread->start();
    }

    void handlePiResult(const QString &result) {
        statusBar->showMessage(result);
    }

private:
    QPushButton *button;
    QLineEdit *input;
    QPushButton *startButton;
    QStatusBar *statusBar;
    QProgressBar *progressBar;
};

// Для обработки сигналов и слотов внутри одного файла
#include "main.moc"

// Точка входа в приложение
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}