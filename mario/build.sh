#!/bin/bash

EXE="main"
CPP="main.cpp"
COMPILER="g++"

# Пути к PDCurses
CURSES_DIR="/home/varya/Desktop/ITbusko/2-curse/mario/lib/PDCurses"
LIB_DIR="$CURSES_DIR/x11"

# Проверка существования библиотеки
if [ ! -f "$LIB_DIR/libpdcurses.a" ]; then
    echo "Ошибка: Библиотека libpdcurses.a не найдена в $LIB_DIR"
    exit 1
fi

# Удаление предыдущего исполняемого файла
[ -f "$EXE" ] && rm "$EXE"

# Компиляция с отладочной информацией
$COMPILER -g -std=c++17 -I"$CURSES_DIR" -L"$LIB_DIR" "$CPP" -o "$EXE" \
    -lpdcurses \
    -lX11 \
    -lXt \
    -lXmu \
    -lXaw \
    -lXext \
    -lSM \
    -lICE \
    -lXpm

# Проверка успешности компиляции и запуск
if [ $? -eq 0 ]; then
    echo "Сборка успешна! Запускаю $EXE..."
    ./"$EXE"
else
    echo "Ошибка компиляции!"
    exit 1
fi
