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

    if command -v gnome-terminal > /dev/null 2>&1; then
        echo "Запуск в gnome-terminal..."
        gnome-terminal --title="Mario Game" -- ./"$EXE" &
        sleep 2
    elif command -v xterm > /dev/null 2>&1; then
        echo "Запуск в xterm..."
        xterm -geometry 250x25 -e "./$EXE" &
        sleep 2
    elif command -v konsole > /dev/null 2>&1; then
        echo "Запуск в konsole..."
        konsole --geometry 250x25 -e "./$EXE" &
        sleep 2
    else
        echo "Запуск в текущем терминале с принудительным изменением размера..."
        # Комбинация методов
        stty cols 250 rows 25
        printf '\033[8;25;250t'
        sleep 2
        ./"$EXE"
    fi

else
    echo "Ошибка компиляции!"
    exit 1
fi
