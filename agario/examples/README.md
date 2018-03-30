## Запуск стратегии под отладчиком
Запустить стратегию под отладчиком (из IDE) можно с помощью `socat`:
- в local_runner в качестве стратегии задаём `socat - TCP-LISTEN:1234`
- запускаем local_runner
- запускаем стратегию из IDE (вместо работы с stdio нужно будет подключиться к tcp порту 1234)

 Установка socat:
 - Linux: `apt-get install socat`
 - OS X: `brew install socat`
 - Windows: можно попробовать найти socat собранный под винду
