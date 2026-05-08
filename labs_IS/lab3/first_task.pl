% Факты

man('Николай').
man('Алексей').
man('Игорь').
man('Павел').
man('Дмитрий').
man('Сергей').

woman('Валентина').
woman('Ольга').
woman('Наталья').
woman('Марина').

married('Николай',   'Валентина').
married('Алексей',   'Ольга').
married('Игорь',     'Наталья').
married('Сергей',    'Марина').

parent('Николай',    'Ольга').
parent('Валентина',  'Ольга').
parent('Николай',    'Игорь').
parent('Валентина',  'Игорь').

parent('Алексей',    'Марина').
parent('Ольга',      'Марина').
parent('Алексей',    'Павел').
parent('Ольга',      'Павел').

parent('Игорь',      'Дмитрий').
parent('Наталья',    'Дмитрий').


% Базовые правила

child(Child, Parent) :-
    parent(Parent, Child).

father(Father, Child) :-
    man(Father),
    parent(Father, Child).

mother(Mother, Child) :-
    woman(Mother),
    parent(Mother, Child).

son(Son, Parent) :-
    man(Son),
    child(Son, Parent).

daughter(Daughter, Parent) :-
    woman(Daughter),
    child(Daughter, Parent).


% Вспомогательные правила

husband(Husband, Wife) :-
    married(Husband, Wife).

wife(Wife, Husband) :-
    married(Husband, Wife).

brother(Brother, Person) :-
    man(Brother),
    father(Father, Brother),
    father(Father, Person),
    mother(Mother, Brother),
    mother(Mother, Person),
    Brother \== Person.

sister(Sister, Person) :-
    woman(Sister),
    father(Father, Sister),
    father(Father, Person),
    mother(Mother, Sister),
    mother(Mother, Person),
    Sister \== Person.


% 6. grandmother - бабушка: мать родителя
grandmother(Grandmother, Grandchild) :-
    mother(Grandmother, Parent),
    parent(Parent, Grandchild).

% 10. nephew - племянник: сын брата
nephew(Nephew, Person) :-
    brother(Brother, Person),
    son(Nephew, Brother).

% 20. wifes_mother - теща: мать жены
wifes_mother(WifesMother, Husband) :-
    wife(Wife, Husband),
    mother(WifesMother, Wife).

% 27. husband_of_daughter - зять: муж дочери
husband_of_daughter(SonInLaw, Parent) :-
    daughter(Daughter, Parent),
    husband(SonInLaw, Daughter).