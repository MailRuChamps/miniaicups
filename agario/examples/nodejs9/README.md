Если вы вдруг хотите дебажить из стратегию, написанную на node.js, то вам необходимо следующее:

* IntelliJ IDEA или аналог (я использовал phpStorm)
* Пару фиксов внести в исходный код local_runner'а:
  *  в файле agario/local_runner/constants.h: 
    
    `const int RESP_TIMEOUT = 5000000;`
  *  в файле agario/local_runner/mainwindow.h:
  
  `void on_error(QString msg) {
    qDebug() << msg;
  }`
* пересобрать local_runner  
* прописать в путь для вашей стратегии:
  
  `node --debug <path_to_strategy>`
  
Всё, теперь можно запускать phpStorm, создавать новый debug-профиль вида Node.js Remote Debug и запустить его.  
Ставим брейкпоинты по в нужных местах, запускаем игру в local_runner'е и наслаждаемся пошаговым дебагом.

Бонусом ещё можно внести правки в agario/local_runner/strategymodal.ui: прописать сразу путь к вашей стратегии,
расставить где нужно radio-button'ы - это все для того, чтобы не менять настройки каждый раз как вы переоткрываете local_runner.

Удачи!
  
