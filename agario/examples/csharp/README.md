Внимание! Данный пример отлично работает с Net Framework 3.5, но не будет работать с чем-то более новым от MS (речь о локальном запуске, при запуске на наших серверах - всё хорошо).  
Поскольку мы не шарписты, мы сами до конца не понимаем, в чём тут дело, но вы точно можете найти всю нужную информацию [здесь](https://social.msdn.microsoft.com/Forums/silverlight/en-US/8bbe87d8-f97f-4258-bce3-dfedd6314e93/consolereadline-not-working-with-net-framework-35?forum=csharpgeneral) :)  
Эпичность происходящего можно ощутить, начав читать наш официальный чат с [этого](https://t.me/aicups/55020) сообщения.

UPD. Дополнение от Ивана Дашкевича (спасибо ему!):

```
public static class FixedConsole
  {
#if !__MonoCS__
    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr GetStdHandle(int nStdHandle);
    private static readonly StreamReader inputStreamReader;

    static FixedConsole()
    {
      var safeFileHandle = new SafeFileHandle(GetStdHandle(-10), false);
      if (safeFileHandle.IsInvalid)
        throw new Exception("Invalid console input handle");
      var inputStream = new FileStream(safeFileHandle, FileAccess.Read);
      inputStreamReader = new StreamReader(inputStream);
    }
#endif

    public static string ReadLine()
    {
#if __MonoCS__
      return Console.ReadLine();
#else
      return inputStreamReader.ReadLine();
#endif
    }
  }
```
