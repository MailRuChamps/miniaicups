сборка под Ubuntu:
```
sudo apt-get install qt5-default &&
git clone https://github.com/MailRuChamps/miniaicups.git &&
cd miniaicups/agario/local_runner/ &&
qmake local_runner.pro &&
make
```
Запуск через `./local_runner`

сборка под MacOS с помощью brew:
```
brew install qt5 &&
export PATH="/usr/local/opt/qt/bin:$PATH" >> ~/.bash_profile &&
. ~/.bash_profile &&
git clone https://github.com/MailRuChamps/miniaicups.git &&
cd miniaicups/agario/local_runner/ &&
qmake local_runner.pro &&
make
```
Запуск через `./local_runner.app/Contents/MacOS/local_runner`
