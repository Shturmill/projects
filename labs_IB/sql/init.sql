-- Лабораторная работа №1, вариант 10
-- Ручная подготовка базы данных.
-- Создание базы (выполнять отдельно, вне базы lab1):
-- CREATE DATABASE lab1;

CREATE TABLE IF NOT EXISTS users (
    id                   SERIAL PRIMARY KEY,
    username             VARCHAR(50) NOT NULL UNIQUE,
    password             VARCHAR(100) NOT NULL DEFAULT '',
    blocked              BOOLEAN NOT NULL DEFAULT FALSE,
    restrictions_enabled BOOLEAN NOT NULL DEFAULT FALSE
);

-- суперпользователь ADMIN с пустым паролем
INSERT INTO users (username, password)
SELECT 'ADMIN', ''
WHERE NOT EXISTS (SELECT 1 FROM users WHERE username = 'ADMIN');
