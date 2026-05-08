current_year(2026).

% athlete(фамилия, год рождения, пол, страна)
athlete('Иванова',  1998,'жен','Россия').
athlete('Ахметов',  1994,'муж','Казахстан').
athlete('Петров',   1992,'муж','Беларусь').
athlete('Смит',	    2000,'жен','США').
athlete('Мюллер',   1997,'муж','Германия').
athlete('Мартен',   1999,'жен','Франция').
athlete('Росси',    1996,'муж','Италия').
athlete('Лехтинен', 2001,'жен','Финляндия').
athlete('Браун',    1995,'муж','Великобритания').

% event(код мероприятия, вид, дата, место)
event(1,'Летний турнир', 	   date(2024,7,15),'Париж').
event(2,'Кубок столицы', 	   date(2024,9,3), 'Лондон').
event(3,'Гран-при',      	   date(2025,5,21),'Токио').
event(4,'Международный старт', date(2025,8,11),'Лос-Анджелес').

% result(фамилия, код мероприятия, высота прыжка, место)
result('Иванова', 1, 1.98, 2).
result('Смит',    1, 2.01, 1).
result('Мартен',  1, 1.95, 3).
result('Ахметов', 2, 2.30, 1).
result('Петров',  3, 2.28, 1).
result('Росси',	  4, 2.31, 1).

% Кто из спортсменок выступал на указанном мероприятии
% (вывести фамилию и страну)
question1(Kind,Date,Place):-
    event(Id,Kind,Date,Place),
    result(Surname,Id,_,_),
    athlete(Surname,_, 'жен', Country),
    write(Surname), write(' - '), writeln(Country),
    fail.


% Занимали ли указанное призовое место спортсмены
% старше определенного возраста
% (вывести фамилию и год рождения)
question2(PrizePlace,MinAge):-
    current_year(CurrentYear),
    athlete(Surname,Birth,_,_),
    Age is CurrentYear - Birth,
    Age > MinAge,
    result(Surname,_,_,PrizePlace),
    write(Surname), write(' - '), writeln(Birth),
    fail.

% Вывести данные спортсменов из заданной страны
% взявших указанную высоту на определенном спортивном мероприятии
question3(Country,Height,Kind,Date,Place):-
    event(Id,Kind,Date,Place),
    result(Surname,Id,Height,_),
    athlete(Surname,Birth,Sex,Country),
    write(Surname), write(' - '),
    write(Birth), write(' - '),
    write(Sex), write(' - '),
    writeln(Country),
    fail.

% Какие еще спортивные мероприятия проводились
% в том же году что и указанное
question4(Kind,date(Year,Month,Day),Place):-
    event(Id,Kind,date(Year,Month,Day),Place),
    event(OtherId,OtherKind,OtherDate,OtherPlace),
    Id \== OtherId,
    OtherDate = date(Year,_,_),
    write(OtherKind), write(' - '),
    write(OtherDate), write(' - '),
    writeln(OtherPlace),
    fail.