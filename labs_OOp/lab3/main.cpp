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

// Класс для вычисления числа π в отдельном потоке
class PiThread : public QThread {
    Q_OBJECT
public:
    PiThread(int iterations, QObject *parent = nullptr)
        : QThread(parent), iterations(iterations), completed_iterations(0), stop_flag(false) {}

    std::atomic<int> completed_iterations; // Количество завершённых итераций
    std::atomic<bool> stop_flag;           // Флаг для остановки вычислений

signals:
    void resultReady(const QString &result); // Сигнал для передачи результата

protected:
    void run() override {
        double sum = 0.0;
        int total_iterations = iterations;

        // Параллельное вычисление π с использованием OpenMP
        #pragma omp parallel for reduction(+:sum)
        for (int k = 0; k < total_iterations; ++k) {
            // Если флаг остановки установлен, пропускаем вычисления
            if (!stop_flag.load(std::memory_order_relaxed)) {
                double term = (1.0 / pow(16, k)) *
                              (4.0 / (8 * k + 1) - 2.0 / (8 * k + 4) -
                               1.0 / (8 * k + 5) - 1.0 / (8 * k + 6));
                sum += term;
            }
            completed_iterations.fetch_add(1, std::memory_order_relaxed); // Обновляем прогресс
        }

        // Формируем результат в зависимости от остановки
        std::ostringstream oss;
        if (!stop_flag.load()) {
            oss << std::fixed << std::setprecision(100) << "Значение π: " << sum;
        } else {
            oss << "Вычисления остановлены на итерации " << completed_iterations.load();
        }
        emit resultReady(QString::fromStdString(oss.str()));
    }

private:
    int iterations; // Количество итераций
};

// Класс основного окна приложения
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Вычисление числа π");

        // Создание меню на русском языке
        QMenu *fileMenu = menuBar()->addMenu("Файл");
        QMenu *editMenu = menuBar()->addMenu("Правка");
        QMenu *helpMenu = menuBar()->addMenu("Справка");

        QAction *openAction = new QAction("Открыть", this);
        openAction->setShortcut(QKeySequence::Open);
        fileMenu->addAction(openAction);

        QAction *saveAction = new QAction("Сохранить", this);
        saveAction->setShortcut(QKeySequence::Save);
        fileMenu->addAction(saveAction);

        QAction *exitAction = new QAction("Выход", this);
        exitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
        fileMenu->addAction(exitAction);

        connect(openAction, &QAction::triggered, this, &MainWindow::onOpen);
        connect(saveAction, &QAction::triggered, this, &MainWindow::onSave);
        connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

        // Настройка интерфейса
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        input = new QLineEdit(this);
        input->setPlaceholderText("Введите количество итераций");
        layout->addWidget(input, 0, Qt::AlignCenter);

        startButton = new QPushButton("Старт", this);
        layout->addWidget(startButton, 0, Qt::AlignCenter);

        stopButton = new QPushButton("Стоп", this);
        stopButton->setEnabled(false);
        layout->addWidget(stopButton, 0, Qt::AlignCenter);

        progressBar = new QProgressBar(this);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->setAlignment(Qt::AlignCenter);
        layout->addWidget(progressBar, 0, Qt::AlignCenter);

        layout->addStretch();

        statusBar = new QStatusBar(this);
        setStatusBar(statusBar);
        statusBar->showMessage("Готово");

        connect(startButton, &QPushButton::clicked, this, &MainWindow::startPiCalculation);
        connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopPiCalculation);
    }

private slots:
    void onOpen() {
        QMessageBox::information(this, "Открыть", "Вы выбрали пункт 'Открыть'.");
    }

    void onSave() {
        QMessageBox::information(this, "Сохранить", "Вы выбрали пункт 'Сохранить'.");
    }

    void onExit() {
        QMessageBox::information(this, "Выход", "Вы выбрали пункт 'Выход'.");
        close();
    }

    void startPiCalculation() {
        bool ok;
        int iterations = input->text().toInt(&ok);
        if (!ok || iterations <= 0) {
            statusBar->showMessage("Ошибка: введите положительное число итераций");
            return;
        }

        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        progressBar->setValue(0);
        progressBar->setStyleSheet("");

        thread = new PiThread(iterations, this);
        connect(thread, &PiThread::resultReady, this, &MainWindow::handlePiResult);
        connect(thread, &PiThread::finished, thread, &QObject::deleteLater);

        connect(thread, &PiThread::finished, this, [this]() {
            if (!thread->stop_flag.load()) {
                progressBar->setValue(100);
                progressBar->setStyleSheet("QProgressBar::chunk { background-color: green; }");
            } else {
                progressBar->setStyleSheet("");
            }
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
        });

        QTimer *timer = new QTimer(this);
        timer->setInterval(100);
        connect(timer, &QTimer::timeout, this, [this, iterations]() {
            if (thread && !thread->stop_flag.load()) {
                int completed = thread->completed_iterations.load(std::memory_order_relaxed);
                int progress = static_cast<int>((completed * 100.0) / iterations);
                if (progress < 100) {
                    progressBar->setValue(progress);
                }
            }
        });
        timer->start();

        connect(thread, &PiThread::finished, timer, &QTimer::stop);
        connect(thread, &PiThread::finished, timer, &QObject::deleteLater);

        thread->start();
        statusBar->showMessage("Вычисления начались...");
    }

    void stopPiCalculation() {
        if (thread) {
            thread->stop_flag.store(true, std::memory_order_relaxed);
            thread->wait(); // Ждём завершения потока
            statusBar->showMessage("Вычисления остановлены");
            progressBar->setStyleSheet("");
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
        }
    }

    void handlePiResult(const QString &result) {
        statusBar->showMessage(result);
    }

private:
    QLineEdit *input;
    QPushButton *startButton;
    QPushButton *stopButton;
    QStatusBar *statusBar;
    QProgressBar *progressBar;
    PiThread *thread = nullptr;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}