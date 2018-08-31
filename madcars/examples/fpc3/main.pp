program main;

const
  COMMANDS: array[0..2] of string = (
    'left', 'right', 'stop'
    );

var
  Z: string;
  Cmd: string;

begin
  while True do
  begin
    ReadLn(Z);
    Cmd := COMMANDS[Random(2)];
    WriteLn('{"command": "' + Cmd + '", "debug": "' + Cmd + '"}');
  end;
end.