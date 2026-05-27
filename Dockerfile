# Базовый образ с MSYS2
FROM msys2/msys2:latest

# Обновляем пакеты и устанавливаем зависимости
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-conan \
    mingw-w64-ucrt-x86_64-make \
    git \
    base-devel

# Создаём рабочую директорию
WORKDIR /app

# Копируем файлы проекта
COPY . .

# Настраиваем прокси (если нужно в企业内部)
# ENV http_proxy=http://tungiia:911@proxy.bolid.ru:3128
# ENV https_proxy=http://tungiia:911@proxy.bolid.ru:3128

# Собираем проект
RUN chmod +x build.sh && ./build.sh

# Открываем порт
EXPOSE 8080

# Запускаем сервер
CMD ["./build/encoder_project.exe"]
