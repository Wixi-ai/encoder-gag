FROM msys2/msys2:latest

ENV http_proxy=http://tungiia:911@proxy.bolid.ru:3128
ENV https_proxy=http://tungiia:911@proxy.bolid.ru:3128

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-conan \
    mingw-w64-ucrt-x86_64-make && \
    pacman -Scc --noconfirm

WORKDIR /app
COPY . .

RUN chmod +x build.sh && ./build.sh

EXPOSE 8080
CMD ["./build/encoder_project.exe"]
