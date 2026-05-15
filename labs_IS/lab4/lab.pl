% Факты

man('Григорий').
man('Пётр').
man('Михаил').
man('Игорь').
man('Иван').
man('Алексей').
man('Павел').
man('Сергей').
man('Владимир').
man('Денис').

woman('Раиса').
woman('Галина').
woman('Наталья').
woman('Ольга').
woman('Анастасия').
woman('Марина').
woman('София').

married('Григорий',   'Раиса').
married('Пётр',       'Галина').
married('Михаил',     'Наталья').
married('Игорь',      'Ольга').
married('Иван',       'Анастасия').
married('Алексей',    'Марина').


% Родители старшего поколения

parent('Григорий',    'Михаил').
parent('Раиса',       'Михаил').

parent('Пётр',        'Наталья').
parent('Галина',      'Наталья').


% Родители среднего поколения

parent('Михаил',      'Игорь').
parent('Наталья',     'Игорь').

parent('Михаил',      'Иван').
parent('Наталья',     'Иван').


% Семья Игоря и Ольги

parent('Игорь',       'Марина').
parent('Ольга',       'Марина').

parent('Игорь',       'Павел').
parent('Ольга',       'Павел').


% Семья Ивана и Анастасии

parent('Иван',        'Сергей').
parent('Анастасия',   'Сергей').

parent('Иван',        'Владимир').
parent('Анастасия',   'Владимир').


% Семья Алексея и Марины

parent('Алексей',     'Денис').
parent('Марина',      'Денис').

parent('Алексей',     'София').
parent('Марина',      'София').



% 2.2 ВАРИАНТ 1
% РЕКУРСИЯ ДЛЯ ГЕНЕАЛОГИЧЕСКОГО ДЕРЕВА

% а) Является ли один человек предком другого

% Базис рекурсии:
% человек является предком другого человека,
% если он является его непосредственным родителем.
predok(Ancestor, Person) :-
    parent(Ancestor, Person).

% Шаг рекурсии:
% человек является предком,
% если он является предком родителя указанного человека.
predok(Ancestor, Person) :-
    parent(Parent, Person),
    predok(Ancestor, Parent).



% б) Количество поколений между предком и потомком

% Если Ancestor является непосредственным родителем Person,
% то их разделяет одно поколение.
predok_generation(Ancestor, Person, 1) :-
    parent(Ancestor, Person).

% Если Ancestor является предком Parent,
% а Parent является родителем Person,
% то число поколений увеличивается на 1.
predok_generation(Ancestor, Person, Generation) :-
    parent(Parent, Person),
    predok_generation(Ancestor, Parent, PreviousGeneration),
    Generation is PreviousGeneration + 1.



% в) Вывод всех предков указанного человека

all_predki(Person) :-
    predok(Ancestor, Person),
    writeln(Ancestor),
    fail.

all_predki(_).



% г) Вывод всех предков с номером поколения

all_predki_generation(Person) :-
    predok_generation(Ancestor, Person, Generation),
    write(Ancestor),
    write(' - поколение '),
    writeln(Generation),
    fail.

all_predki_generation(_).




% 2.4 ВАРИАНТ 9
% ЧИСЛО СОЧЕТАНИЙ C(N,K)
%
% C(N,0) = 1
% C(N,N) = 1
% C(N,K) = C(N-1,K) + C(N-1,K-1)



comb(N, 0, 1) :-
    N >= 0,
    !.

comb(N, N, 1) :-
    N >= 0,
    !.

comb(N, K, Result) :-
    N > 0,
    K > 0,
    K < N,
    N1 is N - 1,
    K1 is K - 1,
    comb(N1, K, Result1),
    comb(N1, K1, Result2),
    Result is Result1 + Result2.



% 2.5 ВАРИАНТ 2
% y = 1 / (1 + x)^2

% Рекурсивное возведение в степень

power(_, 0, 1) :-
    !.

power(X, Degree, Result) :-
    Degree > 0,
    Degree1 is Degree - 1,
    power(X, Degree1, PreviousResult),
    Result is PreviousResult * X.



% Определение знака члена ряда

sign(Number, 1) :-
    1 is Number mod 2,
    !.

sign(_, -1).



% Один член ряда

row_member(Number, X, Result) :-
    Degree is Number - 1,
    power(X, Degree, XDegree),
    sign(Number, Sign),
    Result is Sign * Number * XDegree.



% Сумма первых N членов ряда

function_row(0, _, 0) :-
    !.

function_row(N, X, Result) :-
    N > 0,
    N1 is N - 1,
    function_row(N1, X, PreviousResult),
    row_member(N, X, CurrentMember),
    Result is PreviousResult + CurrentMember.



% Точное значение функции

exact_function(X, Result) :-
    Result is 1 / ((1 + X) * (1 + X)).



% Расчет приближенного и точного значения

calculate_function(N, X) :-
    X > -1,
    X < 1,
    function_row(N, X, Approximate),
    exact_function(X, Exact),
    write('Приближенное значение функции: '),
    writeln(Approximate),
    write('Точное значение функции: '),
    writeln(Exact).

calculate_function(_, _) :-
    writeln('Ошибка: значение X должно быть больше -1 и меньше 1.').



% Ввод данных пользователем

start :-
    write('Введите количество членов ряда N: '),
    read(N),
    write('Введите значение X: '),
    read(X),
    calculate_function(N, X).
