# Устанавливаем терминал (вывод в PNG файл) и размер
set terminal pngcairo enhanced font "arial,10" size 800,600
# Имя выходного файла
set output 'interpolation_plot.png'

# Название графика
set title "График исходной функции и кубического сплайна"
# Подписи осей
set xlabel "x"
set ylabel "f(x)
# Включаем сетку
set grid
# Расположение легенды
set key top right

# Команда для построения графика
plot 'plot_data.dat' using 1:2 with lines linewidth 2 title "Аналитическая f(x)", \
     'plot_data.dat' using 1:3 with lines dashtype 2 linewidth 2 title "Кубический сплайн S(x)", \
     'nodes.dat' using 1:2 with points pointtype 7 pointsize 1.5 title "Узлы интерполяции"

# Сообщение о завершении
print "График сохранен в файл interpolation_plot.png"